#include "../include/cland/cland.h"

char name[] = "cl_minal";

// cl_input proccess
size_t input_id;
size_t input_pid = (size_t) -1;

// root proccess
size_t root_id;
size_t root_pid = (size_t) -1;

// shell proccess
size_t shell_id;
size_t shell_pid = (size_t) -1;

void find_proccess() {
    for (size_t i = 0; i < MAX_COROUTINES; i++) {
        scl_coroutine_send(i, 1, NULL);
    }
}

size_t last_input_count = 0;
size_t last_input_head = 0;
cl_routines *routines;

void cmd_handler(scl_string_t *full_cmd) {
    scl_string_t *target = scl_string_new(); scl_string_cappend(target, " ");
    scl_array_t *args = scl_string_slice(full_cmd, target, 0, (size_t) -1);
    scl_string_destroy(target);
    scl_string_t *cmd = *(scl_string_t **) scl_array_at(args, 0);

    if (scl_string_ccompare(cmd, "exit")) {
        scl_coroutine_send(shell_pid, 3, scl_string_copy(cmd));
        scl_coroutine_yield();
    } else if (scl_string_ccompare(cmd, "clear")) {
        scl_coroutine_send(shell_pid, 3, scl_string_copy(cmd));
        scl_coroutine_sleep(16);
        printf("> ");
    } else if (scl_string_ccompare(cmd, "")) {
        printf("> ");
    } else if (scl_string_ccompare(cmd, "help")) {
        printf("cland minal 0.1\nhelp - show this message\nclear - clear screen\nexit - exit cland\nrout - all proccess info\nkill [pid] - kill proccess\nsummon [module name] - summon module\n> ");
    } else if (scl_string_ccompare(cmd, "rout")) {
        if (!scl_coroutine_find(root_pid)) {
            printf("root proccess is dead...\n");
        } else {
            for (size_t i = 0; i < MAX_COROUTINES; i++) {
                if (scl_coroutine_find(routines[i].pid)) {
                    size_t uptime = (scl_ms() - routines[i].spawn_time) / 1000;
                    printf("proccess: %s, pid: %ld, id: %ld, status: %c, uptime: %ld\n", routines[i].name, routines[i].pid, routines[i].id, routines[i].status, uptime);
                }
            }
        }
        printf("> ");
    } else if (scl_string_ccompare(cmd, "kill") && scl_array_length(args) == 2) {
        scl_coroutine_send(shell_pid, 3, scl_string_copy(full_cmd));
        scl_coroutine_sleep(16);
        printf("> ");
    } else if (scl_string_ccompare(cmd, "summon")) {
        scl_coroutine_send(shell_pid, 3, scl_string_copy(full_cmd));
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

    while (true) {
        scl_coroutine_t *proccess = &scl_coroutines[scl_current_coroutine_pid];

        if (!scl_coroutine_find(input_pid) || !scl_coroutine_find(shell_pid) || !scl_coroutine_find(root_pid)) find_proccess();
        if (scl_coroutine_find(root_pid) && !routines) scl_coroutine_send(root_pid, 3, NULL);

        for (size_t i = 0; i < MAX_MESSAGE; i++) {
            scl_coroutine_message *msg = &proccess->msg[i];

            if (!msg->occupied) continue;
            msg->occupied = false;

            switch (msg->signal) {
                case 0:
                    proccess->status = 'c';
                    scl_string_destroy(buffer);
                    scl_coroutine_yield();
                case 1:
                    scl_coroutine_send(msg->pid, 2, &name);
                    break;
                case 2:
                    char *msg_name = (char *) msg->source;

                    if (strcmp(msg_name, "cl_input") == 0) {
                        input_pid = msg->pid;
                        input_id = msg->sender_id;
                    }
                    if (strcmp(msg_name, "cl_shell") == 0) {
                        shell_pid = msg->pid;
                        shell_id = msg->sender_id;
                    }
                    if (strcmp(msg_name, "cl_root") == 0) {
                        root_pid = msg->pid;
                        root_id = msg->sender_id;
                    }

                    break;
                
                case 3:
                    if (msg->sender_id == input_id) {
                        cl_input_pool *pool = msg->source;

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
                    if (msg->sender_id == root_id) {
                        routines = (cl_routines *) msg->source;
                    }

                    break;
            };
        }

        if (scl_coroutine_find(input_pid)) {
            scl_coroutine_send(input_pid, 3, NULL);
        }

        scl_coroutine_sleep(16);
    }
}