#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "arena.h"
#include "error_handling.h"

int main(void) {
    gLogging = INFO;
    gAsserts = INFO;
    Arena* arena;
    i32*   chunk;
    i32    cap, sz;
    cap = 8;
    sz  = cap;
    CHECK(
        construct_arena(&arena, cap * sizeof(i32)),
        EXIT,
        NULL){};    // Custom ErrorCode checked, no user description written
    errno = EACCES;
    CHECK_ERRNO((void)1, EXIT, "hello", 1, EACCES){};    // Error omitted
    errno = EACCES;
    CHECK_ERRNO(
        (void)1, EXIT, "hello"){};    // Error checked, description written
    mem_acquire_on_arena((void**)&chunk, arena, sz * sizeof(i32));
    for (i32 i = 0; i < sz; ++i) {
        chunk[i] = i * i;
    }
    for (i32 i = 0; i < sz; ++i) {
        printf("%d\n", chunk[i]);
    }
    return 0;
}
