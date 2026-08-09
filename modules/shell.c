#include "../include/cland/cland.h"

char name[] = "cl_shell";

// root proccess
size_t root_id;
size_t root_pid = (size_t) -1;

void find_proccess() {
    for (size_t i = 0; i < MAX_COROUTINES; i++) {
        scl_coroutine_send(i, 1, NULL);
    }
}

void cmd_handler(scl_string_t *full_cmd) {
    scl_string_t *target = scl_string_new(); scl_string_cappend(target, " ");
    scl_array_t *args = scl_string_slice(full_cmd, target, 0, (size_t) -1);
    scl_string_destroy(target);
    scl_string_t *cmd = *(scl_string_t **) scl_array_at(args, 0);
    size_t argc = scl_array_length(args) - 1;

    if (scl_string_ccompare(cmd, "exit")) {
        scl_coroutine_send(root_pid, 0, NULL);
    } else if (scl_string_ccompare(cmd, "clear")) {
        system("clear");
    } else if (scl_string_ccompare(cmd, "kill") && argc == 1) {
        char *proccess = scl_string_cstr(*(scl_string_t **) scl_array_at(args, 1));
        scl_coroutine_t *coroutine = scl_coroutine_find(atoi(proccess));

        if (coroutine) {
            coroutine->status = 'c';
        }

        free(proccess);
    } else if (scl_string_ccompare(cmd, "summon") && argc == 1) {
        char *proccess = scl_string_cstr(*(scl_string_t **) scl_array_at(args, 1));
        scl_coroutine_load(proccess);
        free(proccess);
    }

    for (size_t i = 0; i < scl_array_length(args); i++) {
        scl_string_destroy(*(scl_string_t **) scl_array_at(args, i));
    }
    scl_array_destroy(args);
}

void routine() {
    printf("shell loaded\n");

    while (true) {
        scl_coroutine_t *proccess = &scl_coroutines[scl_current_coroutine_pid];

        if (!scl_coroutine_find(root_pid)) find_proccess();

        for (size_t i = 0; i < MAX_MESSAGE; i++) {
            scl_coroutine_message *msg = &proccess->msg[i];

            if (!msg->occupied) continue;
            msg->occupied = false;

            switch (msg->signal) {
                case 0:
                    proccess->status = 'c';
                    scl_coroutine_yield();
                case 1:
                    scl_coroutine_send(msg->pid, 2, &name);
                    break;
                case 2:
                    char *msg_name = (char *) msg->source;
                    if (strcmp(msg_name, "cl_root") == 0) {
                        root_pid = msg->pid;
                        root_id = msg->sender_id;
                    }
                    break;
                case 3:
                    cmd_handler(msg->source);
            };
        }

        scl_coroutine_sleep(16);
    }
}