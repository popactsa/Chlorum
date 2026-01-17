#include <errno.h>
#include <stdio.h>
#include "auxiliary_utilities.h"
#include "arena.h"

int main(
    [[maybe_unused]] int    argc,
    [[maybe_unused]] char** argv) {
    i32*  ptr;
    i32   sz = 5;
    Error e;
    realloc_dft_arena(sizeof(i32) * sz - 1);
    e = mem_acquire_dft_arena(&ptr, sz);
    if (e.ec) {
        LOG_ERROR(e);
    }
    realloc_dft_arena(sizeof(i32) * sz);
    e = mem_acquire_dft_arena(&ptr, sz);
    for (i32 i = 0; i < sz; ++i) {
        ptr[i] = i;
        printf("%d\n", ptr[i]);
    }
    min(NULL, NULL);
    return 0;
}
