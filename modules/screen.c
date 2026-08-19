#include "../include/snap/snap.h"

char name[] = "snp_screen";
snp_shell_t *shell = NULL;
snp_composer_t *composer = NULL;
snp_screen_t conf;

void routine() {
    printf("screen loaded\n");
    snp_shell_handshake(&shell);
    scl_array_t *screen = scl_array_new(sizeof(snp_draw_t));

    {
        snp_request_t req;
        req.service = 2;
        req.signal = SNP_INFO_A;
        req.source = NULL;
        scl_message_send(shell->pid, 5, &req);
        scl_coroutine_sleep(16);
    }

    while (true) {
        scl_coroutine_t *process = &scl_coroutines[scl_coroutine_current];

        if (composer) {
            if (conf.width != composer->width || conf.height != composer->height) {
                composer->width = conf.width;
                composer->height = conf.height;
            }

            if (scl_array_length(screen) != conf.width * conf.height && shell->in_focus) {
                scl_array_realloc(screen, conf.width * conf.height);
                snp_draw_t temp;
                scl_array_fill(screen, &temp);
            }
        }

        scl_message_t *msg;
        while ((msg = scl_message_pool())) {
            switch (msg->signal) {
                case SNP_KILL_S:
                    scl_array_destroy(screen);
                    process->status = 'c';
                    scl_coroutine_yield();
                case SNP_NAME_A:
                    scl_message_send(msg->pid, SNP_NAME_R, &name);
                    break;
                case SNP_INFO_A:
                    scl_message_send(msg->pid, SNP_INFO_R, &conf);
                    break;
                case SNP_INFO_R:
                    if (shell->process[2].id == msg->id) {
                        composer = (snp_composer_t *) msg->source;
                        conf.width = composer->width;
                        conf.height = composer->height;
                    }
                    break;
                case 5:
                    scl_array_t *data = (scl_array_t *) msg->source;

                    for (size_t i = 0; i < scl_array_length(data); i++) {
                        snp_draw_t *info = (snp_draw_t *) scl_array_at(data, i);

                        uint16_t y = info->y;
                        uint16_t x = info->x;
                        uint16_t index = y * conf.width + x;

                        snp_draw_t *target = (snp_draw_t *) scl_array_at(screen, index);
                        *target = *info;
                    }
                    scl_array_destroy(data);

                    break;
                case 6: // refresh
                    if (!shell->in_focus) break;

                    snp_request_t req;
                    req.service = 2;
                    req.signal = 5;
                    req.source = screen;
                    scl_message_send(shell->pid, 5, &req);
                    scl_coroutine_jump(shell->pid);
                    scl_coroutine_jump(shell->process[2].pid);
                    req.signal = 6;
                    scl_message_send(shell->pid, 5, &req);
                    scl_coroutine_jump(shell->pid);
                    scl_coroutine_jump(shell->process[2].pid);

                    screen = scl_array_new(sizeof(snp_draw_t));

                    break;
            }
        }

        scl_coroutine_sleep(16);
    }
}