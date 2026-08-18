#ifndef SNP_PROTOCOL_H
#define SNP_PROTOCOL_H

#include "../simpcl/simpcl.h"

#define POOL_SIZE 128

typedef enum snp_message_t { // _S is signal, _A is ask, _R is response
    SNP_KILL_S,
    SNP_NAME_A,
    SNP_NAME_R,
    SNP_INFO_A,
    SNP_INFO_R,
} snp_message_t;

typedef struct snp_pool_t {
    char buffer[POOL_SIZE];
    uint16_t head;
    uint16_t size;
    size_t count;
} snp_pool_t;

typedef struct snp_routine_t {
    uint16_t pid;
    size_t id;
    char *name;
    size_t spawn_time;
} snp_routine_t;

// composer
typedef struct snp_color_t {
    uint8_t r, g, b;
} snp_color_t;

typedef struct snp_pixel_t {
    uint8_t up;
    uint8_t down;
} snp_pixel_t;

typedef struct snp_draw_t {
    snp_color_t color;
    uint16_t x, y;
} snp_draw_t;

typedef struct snp_composer_t {
    uint16_t width, height;
} snp_composer_t;

// shell
typedef struct snp_request_t {
    uint16_t service;
    uint16_t signal;
    void *source;
} snp_request_t;

typedef struct snp_shell_t {
    snp_routine_t *routines;
    snp_routine_t *process;
    uint16_t pid;
    bool in_focus;
} snp_shell_t;

static inline void snp_shell_handshake(snp_shell_t **shell) {
    // checar quem é shell
    // enviar comando SNP_NAME_A
    // se for shell guardar pid em array
    // mandar comando child
    // enquanto nao responder tentar reenviar, sleep e checar caixa de mensagens
    // ao sair zerar caixa de mensagens
    bool shell_pid[MAX_COROUTINES];
    memset(&shell_pid, false, sizeof(shell_pid));

    #define MAX_COUNT 32
    size_t count = 0;
    bool connected = false;
    while (!connected) {
        if (count >= MAX_COUNT) {
            scl_coroutine_t *c = &scl_coroutines[scl_coroutine_current];
            c->status = 'c';
            scl_coroutine_yield();
        }

        scl_message_t *msg;
        scl_message_foreach(1, NULL);
        while ((msg = scl_message_pool())) {
            if (msg->signal == SNP_NAME_R && msg->source) {
                if (strcmp((char *) msg->source, "snp_shell") == 0) {
                    shell_pid[msg->pid] = true;
                    continue;
                }
            }
            if (msg->signal != 8 || !msg->source) continue;
            *shell = (snp_shell_t *) msg->source;
            connected = true;
            break;
        }
        scl_coroutine_sleep(16);

        if (count % 4 == 0) {
            for (size_t i = 0; i < MAX_COROUTINES; i++) {
                if (!shell_pid[i]) continue;

                scl_message_send(i, 7, scl_string_from("child"));
            }
        }

        scl_coroutine_sleep(16);
        count++;
    }
    #undef MAX_COUNT

    while (scl_message_pool()); // clear mailbox
    return;
}

#endif