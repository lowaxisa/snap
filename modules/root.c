#include "../include/cland/cland.h"
char name[] = "snp_root";

snp_routine_t routines[MAX_COROUTINES];

void find_process() {
    for (size_t i = 0; i < MAX_COROUTINES; i++) {
        scl_message_send(i, 1, NULL);
    }
}

void routine() {
    printf("root loaded\n");
    scl_coroutine_load("./input.so");
    scl_coroutine_load("./composer.so");
    scl_coroutine_load("./shell.so");

    routines[scl_current_coroutine_pid].pid = scl_current_coroutine_pid;
    routines[scl_current_coroutine_pid].id = scl_coroutines[scl_current_coroutine_pid].id;
    routines[scl_current_coroutine_pid].name = name;
    routines[scl_current_coroutine_pid].spawn_time = scl_ms();

    while (true) {
        scl_coroutine_t *process = &scl_coroutines[scl_current_coroutine_pid];

        for (size_t i = 0; i < MAX_MESSAGE; i++) {
            scl_message_t *msg = &process->msg[i];

            if (!msg->occupied) continue;
            msg->occupied = false;

            switch (msg->signal) {
                case 0:
                    for (size_t j = 0; j < MAX_COROUTINES; j++) {
                        scl_message_send(j, 0, NULL);
                    }
                    process->status = 'c';
                    scl_coroutine_yield();
                case 1:
                    scl_message_send(msg->pid, 2, &name);
                    break;
                case 2: // update process table
                    char *msg_name = (char *) msg->source;

                    for (size_t i = 0; i < MAX_COROUTINES; i++) {
                        if (routines[i].id != msg->sender_id && i == msg->pid && scl_coroutine_find(msg->pid)) {
                            routines[i].pid = msg->pid;
                            routines[i].id = msg->sender_id;
                            routines[i].name = msg_name;
                            routines[i].spawn_time = scl_ms();
                        } else if (!scl_coroutine_find(i)) {
                            routines[i].pid = 0xFFFF;
                        }
                    }

                    break;
                case 5:
                    scl_message_send(msg->pid, 6, &routines);
                    break;
            };
        }

        find_process();
        scl_coroutine_sleep(16);
    }
}