#include "../include/snap/snap.h"

char name[] = "snp_screen";
snp_shell_t *shell = NULL;
snp_composer_t *composer = NULL;

void routine() {
    printf("screen loaded\n");
    snp_shell_handshake(&shell);
    scl_array_t *screen = scl_array_new(sizeof(snp_pixel_t));

    {
        snp_request_t req;
        req.service = 2;
        req.signal = SNP_INFO_A;
        req.source = NULL;
        scl_message_send(shell->pid, 5, &req);
        scl_coroutine_sleep(16);
    }

    while (true) {
        scl_coroutine_t *process = &scl_coroutines[scl_current_coroutine_pid];

        scl_message_t *msg;
        while ((msg = scl_message_pool())) {
            switch (msg->signal) {
                case SNP_KILL_S:
                    process->status = 'c';
                    scl_coroutine_yield();
                case SNP_NAME_A:
                    scl_message_send(msg->pid, SNP_NAME_R, &name);
                    break;
                case SNP_INFO_R:
                    if (shell->process[2].id == msg->id) {
                        composer = (snp_composer_t *) msg->source;
                    }
                    break;
                case 5:
                    break;
            }
        }

        scl_coroutine_sleep(16);
    }
}