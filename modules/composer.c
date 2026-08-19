#include "../include/snap/snap.h"

#include <unistd.h>

typedef struct snp_ansi_t {
    uint8_t r, g, b;
    uint8_t code;
} snp_ansi_t;

snp_ansi_t ansi_palette[16] = {
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

int rgb_to_ansi(snp_color_t color) {
    float current_score = 0.0f;
    int ansi_color = 30;

    for (int i = 0; i < 16; i++) {
        snp_ansi_t ansi = ansi_palette[i];

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

snp_composer_t conf;
const char *pixel_char = "▀";

char name[] = "snp_composer";
void routine() {
    printf("composer loaded\n");
    scl_string_t *buffer = scl_string_new();
    scl_array_t *screen = scl_array_new(sizeof(snp_pixel_t));
    conf.width = 40;
    conf.height = 40;
    
    uint16_t screen_width = conf.width;
    uint16_t screen_height = conf.height / 2;
    while (true) {
        scl_coroutine_t *process = &scl_coroutines[scl_coroutine_current];

        screen_width = conf.width;
        screen_height = conf.height / 2;

        if (scl_array_length(screen) != screen_height * screen_width) {
            scl_array_realloc(screen, screen_height * screen_width);
            snp_pixel_t temp;
            scl_array_fill(screen, &temp);
        }

        scl_message_t *msg;
        while ((msg = scl_message_pool())) {
            switch (msg->signal) {
                case SNP_KILL_S:
                    scl_string_destroy(buffer);
                    scl_array_destroy(screen);
                    process->status = 'c';
                    scl_coroutine_yield();
                case SNP_NAME_A:
                    scl_message_send(msg->pid, SNP_NAME_R, &name);
                    break;
                case SNP_INFO_A:
                    scl_message_send(msg->pid, SNP_INFO_R, &conf);
                    break;
                case 5:
                    scl_array_t *data = (scl_array_t *) msg->source;

                    for (size_t i = 0; i < scl_array_length(data); i++) {
                        snp_draw_t *info = (snp_draw_t *) scl_array_at(data, i);

                        uint16_t y = info->y / 2;
                        uint16_t x = info->x;
                        uint8_t color = rgb_to_ansi(info->color);

                        uint16_t index = y * screen_width + x;
                        if (x >= screen_width || y >= screen_height) continue;

                        snp_pixel_t *pixel = scl_array_at(screen, index);
                        if (info->y % 2 == 0) {
                            pixel->up = color;
                        } else {
                            pixel->down = color;
                        }
                    }
                    scl_array_destroy(data);

                    break;
                case 6:
                    scl_string_clear(buffer);
                    scl_string_cappend(buffer, "\033[H");

                    char temp[32];
                    for (uint16_t line = 0; line < screen_height; line++) {
                        for (uint16_t column = 0; column < screen_width; column++) {
                            // \033[text color;background color
                            snp_pixel_t *pixel = scl_array_at(screen, line * screen_width + column);

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