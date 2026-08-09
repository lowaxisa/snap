#include "include/cland/cland.h"

int main() {
    scl_coroutine_load("./root.so");
    scl_coroutine_init();

    return 0;
}