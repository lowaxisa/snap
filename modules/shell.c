#include "../include/snap/snap.h"

char name[] = "snp_shell";

// system process control
// 0 = root, 1 = input, 2 = composer
snp_routine_t snp_process[3];
bool in_focus = false;
snp_routine_t *routines;
snp_routine_t child[MAX_COROUTINES];
snp_shell_t info;

void child_summon(char *name) {
    uint16_t pid = scl_coroutine_load(name);
    if (pid == 0xFFFF) return;
    uint16_t free_index = 0;

    for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
        if (!scl_coroutine_find(child[i].pid) || !child[i].name) free_index = i;
    }

    child[free_index].pid = pid;
    child[free_index].id = scl_coroutine_find(pid)->id;
    child[free_index].name = NULL;
    child[free_index].spawn_time = scl_ms();
}

snp_routine_t *child_find(uint16_t pid) {
    for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
        if (child[i].pid == pid && child[i].name) return &child[i];
    }
    return NULL;
}

void check_process() {
    if (!routines && scl_coroutine_find(snp_process[0].pid)) {
        scl_message_send(snp_process[0].pid, 5, NULL);
    }

    for (size_t i = 0; i < 3; i++) {
        if (!snp_process[i].name || !scl_coroutine_find(snp_process[i].pid)) {
            scl_message_foreach(1, NULL);
            scl_coroutine_sleep(16);
            break;
        }
    }

    if (routines) {
        for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
            if (!child[i].id || !scl_coroutine_find(child[i].pid)) continue; 
            for (uint16_t j = 0; j < MAX_COROUTINES; j++) {
                if (routines[j].id == child[i].id) child[i].name = routines[j].name;
            }
        }
    }
}

void update_process(scl_message_t *msg) {
    char *msg_name = (char *) msg->source;

    if (strcmp(msg_name, "snp_root") == 0) {
        snp_process[0].name = msg_name;
        snp_process[0].pid = msg->pid;
        snp_process[0].id = msg->sender_id;
        snp_process[0].spawn_time = (size_t) -1;
    }
    if (strcmp(msg_name, "snp_input") == 0) {
        snp_process[1].name = msg_name;
        snp_process[1].pid = msg->pid;
        snp_process[1].id = msg->sender_id;
        snp_process[1].spawn_time = (size_t) -1;
    }
    if (strcmp(msg_name, "snp_composer") == 0) {
        snp_process[2].name = msg_name;
        snp_process[2].pid = msg->pid;
        snp_process[2].id = msg->sender_id;
        snp_process[2].spawn_time = (size_t) -1;
    }
}

// command handler
void ask_focus() {
    for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
        snp_routine_t *r = &routines[i];

        if (!scl_coroutine_find(r->pid) || !r->name) continue;
        if (strcmp(r->name, "snp_shell") == 0 && r->pid != scl_current_coroutine_pid) {
            scl_string_t *temp = scl_string_new();
            scl_string_cappend(temp, "focus false");
            scl_message_send(r->pid, 7, temp);
        }
    }
    in_focus = true;
}

typedef struct command_t {
    char *cmd;
    void (*callback)(scl_message_t *msg, scl_array_t *array);
    uint16_t argc;
} command_t;

// commands functions
void cmd_exit(scl_message_t *msg, scl_array_t *array) {
    scl_message_send(snp_process[0].pid, 0, NULL);
}

void cmd_clear(scl_message_t *msg, scl_array_t *array) {
    system("clear");
}

void cmd_kill(scl_message_t *msg, scl_array_t *array) {
    char *process = scl_string_cstr(*(scl_string_t **) scl_array_at(array, 1));
    snp_routine_t *coroutine = child_find(atoi(process));

    if (coroutine && coroutine->pid != scl_current_coroutine_pid) {
        scl_coroutine_t *r = scl_coroutine_find(coroutine->pid);
        r->status = 'c';
        coroutine->name = NULL;
    }

    free(process);
}

void cmd_summon(scl_message_t *msg, scl_array_t *array) {
    char *process = scl_string_cstr(*(scl_string_t **) scl_array_at(array, 1));
    child_summon(process);
    free(process);
}

void cmd_focus(scl_message_t *msg, scl_array_t *array) {
    scl_string_t *s = *(scl_string_t **) scl_array_at(array, 1);

    if (scl_string_ccompare(s, "true")) {
        ask_focus();
    } else {
        in_focus = false;
    }
}

void cmd_child(scl_message_t *msg, scl_array_t *array) {
    for (uint16_t i = 0; i < MAX_COROUTINES; i++) {
        if (child[i].id == msg->sender_id) {
            scl_message_send(msg->pid, 8, &info);
        }
    }
}

// commands table
command_t commands[] = {
    {"exit", cmd_exit, 0},
    {"clear", cmd_clear, 0},
    {"kill", cmd_kill, 1},
    {"summon", cmd_summon, 1},
    {"focus", cmd_focus, 1},
    {"child", cmd_child, 0},
};
#define COMMANDS_LENGTH (sizeof(commands) / sizeof(commands[0]))

void cmd_handler(scl_message_t *msg) {
    scl_string_t *full_cmd = (scl_string_t *) msg->source;

    scl_array_t *args = scl_string_cslice(full_cmd, " ", 0, (size_t) -1);
    scl_string_t *cmd = *(scl_string_t **) scl_array_at(args, 0);
    size_t argc = scl_array_length(args) - 1;

    for (size_t i = 0; i < COMMANDS_LENGTH; i++) {
        if (scl_string_ccompare(cmd, commands[i].cmd) && commands[i].argc == argc) commands[i].callback(msg, args);
    }

    for (size_t i = 0; i < scl_array_length(args); i++) {
        scl_string_destroy(*(scl_string_t **) scl_array_at(args, i));
    }
    scl_array_destroy(args);
    scl_string_destroy(full_cmd);
}

void routine() {
    printf("shell loaded\n");
    info.routines = child;
    info.shell_pid = scl_current_coroutine_pid;
    info.process = snp_process;

    // start child table
    child[0].id = scl_coroutines[scl_current_coroutine_pid].id;
    child[0].name = name;
    child[0].pid = scl_current_coroutine_pid;
    child[0].spawn_time = scl_ms();

    child_summon("./minal.so");

    while (true) {
        scl_coroutine_t *process = &scl_coroutines[scl_current_coroutine_pid];
        check_process();

        scl_message_t *msg;
        while (msg = scl_message_pool()) {
            switch (msg->signal) {
                case KILL:
                    process->status = 'c';
                    scl_coroutine_yield();
                case ASK_NAME:
                    scl_message_send(msg->pid, 2, &name);
                    break;
                case RESP_NAME:
                    update_process(msg);
                    break;
                case 5:
                    if (!in_focus) break;
                    snp_request_t *req = (snp_request_t *) msg->source;

                    if (scl_coroutine_find(snp_process[req->service].pid)) {
                        uint16_t temp = scl_current_coroutine_pid;
                        scl_current_coroutine_pid = msg->pid;
                        scl_message_send(snp_process[req->service].pid, req->signal, req->source);
                        scl_current_coroutine_pid = temp;
                    }

                    break;
                case 6:
                    if (msg->sender_id == snp_process[0].id) {
                        routines = (snp_routine_t *) msg->source;
                        ask_focus();
                    }
                    break;
                case 7:
                    cmd_handler(msg);
                    break;
            };
        }

        scl_coroutine_sleep(16);
    }
}