#include "../include/cland/cland.h"

#include <unistd.h>

#define WIDTH 60
#define HEIGHT (40 / 2)

typedef struct cl_ansi_color {
    uint8_t r, g, b;
    uint8_t code;
} cl_ansi_color;

cl_ansi_color ansi_palette[16] = {
    {0,   0,   0,   30}, // black
    {170, 0,   0,   31}, // red
    {0,   170, 0,   32}, // green
    {170, 85,  0,   33}, // yellow
    {0,   0,   170, 34}, // blue
    {170, 0,   170, 35}, // magenta
    {0,   170, 170, 36}, // cyan
    {170, 170, 170, 37}, // white (light gray)
    {85,  85,  85,  90}, // bright black (gray)
    {255, 85,  85,  91}, // bright red
    {85,  255, 85,  92}, // bright green
    {255, 255, 85,  93}, // bright yellow
    {85,  85,  255, 94}, // bright blue
    {255, 85,  255, 95}, // bright magenta
    {85,  255, 255, 96}, // bright cyan
    {255, 255, 255, 97}, // bright white
};

float channel_score(float diff) {
    if (diff == 0) return 1.0f;
    return 1.0f / (float) (diff * diff);
}

int rgb_to_ansi(cl_composer_color color) {
    float current_score = 0.0f;
    int ansi_color = 30;

    for (int i = 0; i < 16; i++) {
        cl_ansi_color ansi = ansi_palette[i];

        float diff_r = ansi.r - color.r;
        float diff_g = ansi.g - color.g;
        float diff_b = ansi.b - color.b;

        float score = channel_score(diff_r) + channel_score(diff_g) + channel_score(diff_b);
        if (score > current_score) {
            current_score = score;
            ansi_color = ansi.code;
        }
    }

    return ansi_color;
}

cl_composer_pixel screen[WIDTH * HEIGHT];
const char *pixel_char = "▀";

char name[] = "cl_composer";
void routine() {
    printf("composer loaded\n");
    scl_string_t *buffer = scl_string_new();
    
    while (true) {
        scl_coroutine_t *proccess = &scl_coroutines[scl_current_coroutine_pid];

        for (size_t i = 0; i < MAX_MESSAGE; i++) {
            scl_coroutine_message *msg = &proccess->msg[i];

            if (!msg->occupied) continue;
            msg->occupied = false;

            switch (msg->signal) {
                case 0:
                    scl_string_destroy(buffer);
                    proccess->status = 'c';
                    scl_coroutine_yield();
                case 1:
                    scl_coroutine_send(msg->pid, 2, &name);
                    break;
                case 3:
                    scl_array_t *data = (scl_array_t *) msg->source;

                    for (size_t i = 0; i < scl_array_length(data); i++) {
                        cl_composer_draw *info = (cl_composer_draw *) scl_array_at(data, i);

                        uint16_t y = info->y / 2;
                        uint16_t x = info->x;
                        uint8_t color = rgb_to_ansi(info->color);

                        uint16_t index = y * WIDTH + x;
                        if (x >= WIDTH || y >= HEIGHT) continue;

                        if (info->y % 2 == 0) {
                            screen[index].up = color;
                        } else {
                            screen[index].down = color;
                        }
                    }
                    scl_array_destroy(data);

                    break;
                case 4:
                    scl_string_clear(buffer);
                    scl_string_cappend(buffer, "\033[H");

                    char temp[32];
                    for (uint16_t line = 0; line < HEIGHT; line++) {
                        for (uint16_t column = 0; column < WIDTH; column++) {
                            // \033[text color;background color
                            cl_composer_pixel *pixel = &screen[line * WIDTH + column];

                            scl_string_cappend(buffer, "\033[");
                            snprintf(temp, sizeof(temp), "%d", pixel->up);
                            scl_string_cappend(buffer, temp);
                            scl_string_cappend(buffer, ";");
                            snprintf(temp, sizeof(temp), "%d", pixel->down + 10);
                            scl_string_cappend(buffer, temp);
                            scl_string_cappend(buffer, "m");
                            scl_string_cappend(buffer, pixel_char);
                        }
                        scl_string_cappend(buffer, "\033[0m\n");
                    }
                    scl_string_print(buffer);

                    break;
            };
        }

        scl_coroutine_sleep(16);
    }
}