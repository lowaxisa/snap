#include "../include/cland/cland.h"

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

void input_raw_enable(void);
void input_raw_disable(void);

snp_pool_t pool;

char name[] = "snp_input";
void routine() {
    printf("input loaded\n");
    input_raw_enable();
    pool.size = POOL_SIZE;
    pool.head = 0;
    pool.count = 0;
    
    while (true) {
        scl_coroutine_t *process = &scl_coroutines[scl_current_coroutine_pid];

        for (size_t i = 0; i < MAX_MESSAGE; i++) {
            scl_message_t *msg = &process->msg[i];

            if (!msg->occupied) continue;
            msg->occupied = false;

            switch (msg->signal) {
                case 0:
                    input_raw_disable();
                    process->status = 'c';
                    scl_coroutine_yield();
                case 1:
                    scl_message_send(msg->pid, 2, &name);
                    break;
                case 3:
                    scl_message_send(msg->pid, 6, &pool);
                    break;
            };
        }

        int c = getchar();
        if (c != EOF) {
            pool.buffer[pool.head] = c;
            pool.head = (pool.head + 1) % pool.size;
            if (!pool.head) pool.count++;
        } else {
            clearerr(stdin);
        }

        scl_coroutine_sleep(16);
    }
}

static struct termios orig_termios;
static bool raw_enabled = false;

void input_raw_enable(void) {
    if (raw_enabled) return;

    tcgetattr(STDIN_FILENO, &orig_termios);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    raw_enabled = true;
}

void input_raw_disable(void) {
    if (!raw_enabled) return;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);

    raw_enabled = false;
}