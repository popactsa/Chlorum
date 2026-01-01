#include <stdio.h>
#include "arena.h"
#include "error_handling.h"

int main(void) {
    gLogging = INFO;
    Arena_t* arena;
    i32      arena_cap = 128 * alignof(i32);
    CHECK(construct_arena(&arena, arena_cap), EXIT, "Failed arena allocation");
    i32* where;
    i32  sz = 4 * alignof(i32);
    mem_acquire_on_arena((void**)&where, arena, sz * sizeof(i32));
    return 0;
}
