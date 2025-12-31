#include "arena.h"
#include <stdlib.h>
#include "error_handling.h"

Arena_t*   arenas[MAXARENAS] = {0};
static i32 arenas_cnt        = 0;

ErrorCode_t construct_arena(
    Arena_t** result,
    const i32 sz) {
    if (CHECK_ANY(
            arenas_cnt == MAXARENAS || result == NULL,
            WARN,
            "Can't construct arena")) {
        return CONTRV;
    }
    Arena_t* arena = malloc(sizeof(Arena_t));
    arena->begin   = malloc(sz);
    if (!arena->begin) {
        return MEMALF;
    }
    arena->end = (char*)arena->begin + sz;

    arenas[arenas_cnt] = arena;
    if (arenas_cnt == 0) {
        CHECK_ANY(atexit(destruct_arenas), ERROR, NULL);
    }
    ++arenas_cnt;
    *result = arena;
    return OK;
}

void destruct_arenas(void) {
    for (int i = 0; i < MAXARENAS; ++i) {
        free(arenas[i]->begin);
        free(arenas[i]);
    }
}
