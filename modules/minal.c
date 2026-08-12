#include "../include/snap/snap.h"

char name[] = "snp_minal";

snp_shell_t *shell = NULL;

size_t last_input_count = 0;
size_t last_input_head = 0;

void cmd_handler(scl_string_t *full_cmd) {
    scl_string_t *target = scl_string_new(); scl_string_cappend(target, " ");
    scl_array_t *args = scl_string_slice(full_cmd, target, 0, (size_t) -1);
    scl_string_destroy(target);
    scl_string_t *cmd = *(scl_string_t **) scl_array_at(args, 0);

    if (scl_string_ccompare(cmd, "exit")) {
        scl_message_send(shell->shell_pid, 7, scl_string_copy(cmd));
        scl_coroutine_yield();
    } else if (scl_string_ccompare(cmd, "clear")) {
        scl_message_send(shell->shell_pid, 7, scl_string_copy(cmd));
        scl_coroutine_sleep(16);
        printf("> ");
    } else if (scl_string_ccompare(cmd, "")) {
        printf("> ");
    } else if (scl_string_ccompare(cmd, "help")) {
        printf("snap minal 0.1\nhelp - show this message\nclear - clear screen\nexit - exit cland\nrout - all process info\nkill [pid] - kill process\nsummon [module name] - summon module\n> ");
    } else if (scl_string_ccompare(cmd, "rout")) {
        for (size_t i = 0; i < MAX_COROUTINES; i++) {
            if (scl_coroutine_find(shell->routines[i].pid) && shell->routines[i].name) {
                size_t uptime = (scl_ms() - shell->routines[i].spawn_time) / 1000;
                printf("process: %s, pid: %ld, id: %ld, uptime: %ld\n", shell->routines[i].name, shell->routines[i].pid, shell->routines[i].id, uptime);
            }
        }
        printf("> ");
    } else if (scl_string_ccompare(cmd, "kill") && scl_array_length(args) == 2) {
        scl_message_send(shell->shell_pid, 7, scl_string_copy(full_cmd));
        scl_coroutine_sleep(16);
        printf("> ");
    } else if (scl_string_ccompare(cmd, "summon")) {
        scl_message_send(shell->shell_pid, 7, scl_string_copy(full_cmd));
        scl_coroutine_sleep(16);
        printf("> ");
    } else {
        printf("command not found...\n> ");
    }

    for (size_t i = 0; i < scl_array_length(args); i++) {
        scl_string_destroy(*(scl_string_t **) scl_array_at(args, i));
    }
    scl_array_destroy(args);
}

void routine() {
    printf("minal loaded\n> ");
    scl_string_t *buffer = scl_string_new();
    snp_shell_handshake(&shell);

    while (true) {
        scl_coroutine_t *process = &scl_coroutines[scl_current_coroutine_pid];

        scl_message_t *msg;
        while ((msg = scl_message_pool())) {
            switch (msg->signal) {
                case KILL:
                    process->status = 'c';
                    scl_string_destroy(buffer);
                    scl_coroutine_yield();
                case ASK_NAME:
                    scl_message_send(msg->pid, 2, &name);
                    break;
                case 6:
                    if (msg->sender_id == shell->process[1].id) {
                        snp_pool_t *pool = msg->source;

                        if (pool->count != last_input_count) {
                            last_input_count = pool->count;
                            last_input_head = 0;
                        }

                        for (size_t i = last_input_head; i < pool->head; i++) {
                            char c = pool->buffer[i];

                            switch (c) {
                                case 10:
                                case 13: // enter
                                    printf("\n");
                                    cmd_handler(buffer);
                                    scl_string_clear(buffer);
                                    break;
                                case 8:
                                case 127: // backspace
                                    if (scl_array_length(buffer->source)) {
                                        printf("\b \b");
                                        scl_array_pop(buffer->source, NULL);
                                    }
                                    break;
                                default:
                                    printf("%c", c);
                                    scl_array_push(buffer->source, &c);
                                    break;
                            }
                        }

                        last_input_head = pool->head;
                    }
                    break;
            };
        }

        snp_request_t input_req;
        input_req.service = 1;
        input_req.signal = 3;
        input_req.source = NULL;
        scl_message_send(shell->shell_pid, 5, &input_req);

        scl_coroutine_sleep(16);
    }
}