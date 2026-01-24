#include "arena.h"
#include <stdlib.h>
#include "auxiliary_utilities.h"

static Arena dft_arena        = {0};
static bool  dft_arena_in_use = false;

Error construct_arena(
    Arena*   arena,
    const i32 cap) {
    {
        bool pre_ok  = true;
        pre_ok      &= arena != NULL;
        pre_ok      &= cap >= 0;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    char* maybe_arena_begin;
    maybe_arena_begin = malloc(cap);
    if (!maybe_arena_begin) {
        return (Error){.ec = MALLOC, .lvl = WARN};
    }
    *arena = (Arena){
        .begin = maybe_arena_begin,
        .cap = cap,
        .sz = 0
    };
    return (Error){0};
}

void destruct_arena(Arena* arena) {
    (void)mem_release_arena(arena);
    free(arena);
}

Error realloc_arena(
    Arena*    arena,
    const i32 new_cap) {
    void* new_begin;
    {
        bool pre_ok  = true;
        pre_ok      &= arena != NULL;
        pre_ok      &= new_cap >= 0;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    new_begin = realloc(arena->begin, new_cap);
    if (!new_begin) {
        mem_release_arena(arena);
        return (Error){.ec = MALLOC, .lvl = ERROR};
    }
    *arena = (Arena){
        .begin = new_begin,
        .cap = new_cap,
        .sz = min_i32(arena->sz, new_cap)
    };
    return (Error){0};
}

Error mem_acquire_arena_void(
    void**    acquired,
    Arena*    arena,
    const i32 sz) {
    {
        bool pre_ok  = true;
        pre_ok      &= acquired != NULL;
        pre_ok      &= arena != NULL;
        pre_ok      &= sz >= 0;
        pre_ok      &= arena->sz >= 0;
        pre_ok      &= arena->cap >= arena->sz + sz;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    if (sz == 0) {
        *acquired = NULL;
        return (Error){.ec   = PRE,
                       .lvl  = WARN,
                       .desc = "Trying to acquire 0 bytes on arena"};
    }
    *acquired  = (void*)(arena->begin + arena->sz);
    arena->sz += sz;
    return (Error){0};
}

Error mem_release_arena(Arena* arena) {
    {
        bool pre_ok  = true;
        pre_ok      &= arena != NULL;
        if (!pre_ok) {
            return (Error){.ec = PRE, .lvl = WARN};
        }
    }
    free(arena->begin);
    *arena = (Arena) {
        .begin = NULL,
        .cap = 0,
        .sz = 0
    };
    return (Error){0};
}

Error mem_acquire_dft_arena_void(
    void**    acquired,
    const i32 sz) {
    return mem_acquire_arena_void(acquired, &dft_arena, sz);
}

Error realloc_dft_arena(const i32 new_cap) {
    return realloc_arena(&dft_arena, new_cap);
}

Error mem_release_dft_arena(void) {
    return mem_release_arena(&dft_arena);
}

inline static void destruct_dft_arena(void) {
    free(dft_arena.begin);
}
