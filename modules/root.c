#include "../include/snap/snap.h"
char name[] = "snp_root";

snp_routine_t routines[MAX_COROUTINES];

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

        scl_message_t *msg;
        while (msg = scl_message_pool()) {
            switch (msg->signal) {
                case KILL:
                    scl_message_foreach(KILL, NULL);
                    process->status = 'c';
                    scl_coroutine_yield();
                case ASK_NAME:
                    scl_message_send(msg->pid, 2, &name);
                    break;
                case RESP_NAME: // update process table
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

        scl_message_foreach(ASK_NAME, NULL);
        scl_coroutine_sleep(16);
    }
}