#include "../include/simpcl/coroutine.h"

sigjmp_buf scl_context_jmp;
ucontext_t scl_context; // main context
uint16_t scl_current_coroutine_pid = 0xFFFF;
uint16_t scl_next_coroutine_pid = 0xFFFF;
scl_coroutine_t scl_coroutines[MAX_COROUTINES];
void *scl_coroutines_handle[MAX_COROUTINES]; // for modules .so
bool scl_coroutine_inited = false;
size_t scl_coroutine_count = 0; // global id count for routines

// helpers
scl_coroutine_t *scl_coroutine_find(uint16_t pid) { // return coroutine if is alive
    if (pid >= MAX_COROUTINES) return NULL;

    if (scl_coroutines[pid].stack != NULL && scl_coroutines[pid].status != 'c') {
        return &scl_coroutines[pid];
    }
    return NULL;
}

uint16_t scl_coroutine_unused_pid() {
    for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
        if (!scl_coroutines[i].stack) {
            return i;
        }
    }
    return 0xFFFF;
}

// logic
void scl_coroutine_gc() {
    for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
        scl_coroutine_t *c = &scl_coroutines[i];

        if (c->stack && c->status == 'c' && i != scl_current_coroutine_pid) {
            free(c->stack);
            c->stack = NULL;

            void *handle = scl_coroutines_handle[i];
            if (handle) {
                dlclose(handle);
                scl_coroutines_handle[i] = NULL;
            }
        }
    }
}

bool scl_coroutine_scheduler() {
    bool has_coroutines_alive = false;
    uint16_t curr_index = 0;
    for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
        scl_coroutine_t *c = &scl_coroutines[i];

        if (c->status == 'r' || c->status == 's') {
            has_coroutines_alive = true;
        }

        if (i == scl_current_coroutine_pid) curr_index = i;
    }

    if (!has_coroutines_alive) return false;

    while (true) {
        bool next_coroutine_find = false;
        size_t sleep_time = 0xFFFFFFFF;
        size_t current_time = scl_ms();

        for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
            uint16_t index = (curr_index + i + 1) % MAX_COROUTINES;
            scl_coroutine_t *c = &scl_coroutines[index];
            if (!c->stack) continue;

            if (c->status == 's') {

                if (current_time >= c->wake_at) {
                    c->status = 'r';
                } else {
                    size_t diff = c->wake_at - current_time;
                    sleep_time = (sleep_time > diff) ? diff : sleep_time;
                }
            }

            if (c->status == 'r') {
                next_coroutine_find = true;
                scl_next_coroutine_pid = index;
                break;
            }
        }

        if (next_coroutine_find) break;

        struct timespec time;
        time.tv_sec = sleep_time / 1000;
        time.tv_nsec = (sleep_time % 1000) * 1000000L;
        nanosleep(&time, NULL);
    }

    return true;
}

void scl_coroutine_yield() {
    scl_coroutine_gc();
    if (!scl_coroutine_scheduler()) siglongjmp(scl_context_jmp, 1);

    scl_coroutine_t *curr = scl_coroutine_find(scl_current_coroutine_pid);
    scl_coroutine_t *next = scl_coroutine_find(scl_next_coroutine_pid);
    scl_current_coroutine_pid = scl_next_coroutine_pid;

    if (!curr) {
        swapcontext(&scl_context, &next->ctx);
    }

    if (curr != next) {
        swapcontext(&curr->ctx, &next->ctx);
    }
    return;
}

void scl_coroutine_entry() {
    scl_coroutine_t *c = &scl_coroutines[scl_current_coroutine_pid];
    c->routine();

    c->status = 'c';
    scl_coroutine_yield();
}

uint16_t scl_coroutine_summon(void (*routine)()) {
    uint16_t pid = scl_coroutine_unused_pid();
    scl_coroutine_t *c = &scl_coroutines[pid];

    c->status  = 'r';
    c->wake_at = 0;
    c->stack   = malloc(STACK_SIZE);
    c->routine = routine;
    c->id      = scl_coroutine_count++;

    // config ctx
    getcontext(&c->ctx);

    c->ctx.uc_stack.ss_sp = c->stack;
    c->ctx.uc_stack.ss_size = STACK_SIZE;
    c->ctx.uc_link = NULL;

    makecontext(&c->ctx, scl_coroutine_entry, 0);
    return pid;
}

void scl_coroutine_crash(int signal) {
    (void)signal;
    scl_coroutine_t *routine = &scl_coroutines[scl_current_coroutine_pid];

    routine->status = 'c';

    scl_coroutine_yield();
}

void scl_coroutine_init() {
    int status = sigsetjmp(scl_context_jmp, 1);

    if (status == 1) {
        scl_current_coroutine_pid = 0xFFFF;
        scl_coroutine_gc();
        return;
    }
    
    getcontext(&scl_context);
    if (!scl_coroutine_inited) {
        scl_coroutine_inited = true;

        struct sigaction sa;
        sa.sa_handler = scl_coroutine_crash;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_NODEFER;

        sigaction(SIGSEGV, &sa, NULL);
        sigaction(SIGBUS, &sa, NULL);
        sigaction(SIGFPE, &sa, NULL);
        sigaction(SIGILL, &sa, NULL);
        sigaction(SIGABRT, &sa, NULL);

        scl_coroutine_yield();
    }
}

void scl_coroutine_sleep(size_t ms) {
    scl_coroutine_t *c = &scl_coroutines[scl_current_coroutine_pid];
    c->wake_at = scl_ms() + ms;
    c->status = (c->status != 'c') ? 's' : 'c';
    scl_coroutine_yield();
}

uint16_t scl_coroutine_load(const char *module) {
    void *handle = dlopen(module, RTLD_LAZY);

    if (!handle) {
        printf("[scl_coroutine_load] dlopen error: %s\n", dlerror());
        return 0xFFFF;
    }
    
    dlerror();
    void (*routine)();
    routine = (void (*)()) dlsym(handle, "routine");

    char *error = dlerror();
    if (error != NULL) {
        dlclose(handle);
        return 0xFFFF;
    }

    uint16_t pid = scl_coroutine_summon(routine);
    scl_coroutines_handle[pid] = handle;
    return pid;
}

// message
char scl_message_send(uint16_t pid, uint16_t signal, void *source) { // s = success, f = target is full, n = target is null, y = is you
    if (pid == scl_current_coroutine_pid) return 'y';

    scl_coroutine_t *target = scl_coroutine_find(pid);

    if (!target) return 'n';

    for (uint16_t i = 0; i < MAX_MESSAGE; i++) {
        if (!target->msg[i].occupied) {
            target->msg[i].occupied = true;
            target->msg[i].pid = scl_current_coroutine_pid;
            target->msg[i].signal = signal;
            target->msg[i].source = source;
            target->msg[i].id = scl_coroutines[scl_current_coroutine_pid].id;
            return 's';
        }
    }

    return 'f';
}

scl_message_t *scl_message_pool() {
    scl_coroutine_t *routine = &scl_coroutines[scl_current_coroutine_pid];

    for (uint16_t i = 0; i < MAX_MESSAGE; i++) {
        scl_message_t *msg = &routine->msg[i];

        if (msg->occupied) {
            msg->occupied = false;
            return msg;
        }
    }

    return NULL;
}

void scl_message_foreach(uint16_t signal, void *source) {
    for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
        scl_message_send(i, signal, source);
    }
}