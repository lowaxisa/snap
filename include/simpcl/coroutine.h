#ifndef SCL_COROUTINE_H
#define SCL_COROUTINE_H

#include <ucontext.h>
#include <setjmp.h>
#include <time.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include "misc.h"

#ifndef NULL
#define NULL (void *)0
#endif

#define STACK_SIZE 128 * 1024
#define MAX_COROUTINES 256
#define MAX_MESSAGE 16

typedef struct scl_coroutine_message {
    uint16_t pid; // the coroutine was send
    size_t sender_id;
    uint16_t signal; // for control
    void *source;
    bool occupied;
} scl_coroutine_message;

struct scl_coroutine_t;
typedef struct scl_coroutine_t {
    char status; // r = running, c = closed, s = sleeping
    size_t wake_at; // wake if time > wake_at (in ms)
    ucontext_t ctx;
    void *stack;
    void (*routine)();
    scl_coroutine_message msg[MAX_MESSAGE];
    size_t id;
} scl_coroutine_t;

// export vars
extern sigjmp_buf scl_context_jmp;
extern ucontext_t scl_context; // main context
extern uint16_t scl_current_coroutine_pid;
extern uint16_t scl_next_coroutine_pid;
extern scl_coroutine_t scl_coroutines[MAX_COROUTINES];
extern void *scl_coroutines_handle[MAX_COROUTINES]; // for modules .so
extern bool scl_coroutine_inited;
extern size_t scl_coroutine_count; // global id count for routines

// helpers
scl_coroutine_t *scl_coroutine_find(uint16_t pid);
uint16_t scl_coroutine_unused_pid();

// logic
void scl_coroutine_gc();
bool scl_coroutine_scheduler();
void scl_coroutine_yield();
void scl_coroutine_entry();
uint16_t scl_coroutine_summon(void (*routine)());
void scl_coroutine_crash(int signal);
void scl_coroutine_init();
void scl_coroutine_sleep(size_t ms);
char scl_coroutine_send(uint16_t pid, uint16_t signal, void *source);
uint16_t scl_coroutine_load(const char *module);

#endif