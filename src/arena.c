#include "arena.h"
#include <stdlib.h>
#include "error_handling.h"

ClArenaNode* arena_nodes_head = NULL;
i32          arena_nodes_cnt  = 0;

ErrorCode construct_arena(
    ClArena** result,
    const i32 cap) {
    ClArenaNode* arena_node;
    ClArena*     arena;
    ASSERT(result, ERROR, "Nowhere to return result. NULL received") {
        return CONTRV;
    }
    arena_node = malloc(sizeof(ClArenaNode));
    ASSERT(arena_node, WARN, "Can't allocate arena node") {
        return MEMALF;
    }
    arena        = &arena_node->arena;
    arena->begin = malloc(cap);
    ASSERT(arena->begin, ERROR, "Arena allocation fail") {
        free(arena_node);
        return MEMALF;
    }
    arena->cap       = cap;
    arena->sz        = 0;
    arena_node->next = arena_nodes_head;
    arena_nodes_head = arena_node;

    if (arena_nodes_cnt == 0) {
        CHECK(atexit(destruct_arenas), EXIT, NULL) {}
    }
    ++arena_nodes_cnt;
    *result = arena;
    return OK;
}

void destruct_arenas(void) {
    ClArenaNode* next;
    while (arena_nodes_cnt) {
        next = arena_nodes_head->next;
        if (arena_nodes_head->arena.begin) {
            free(arena_nodes_head->arena.begin);
        }
        free(arena_nodes_head);
        arena_nodes_head = next;
        --arena_nodes_cnt;
    }
}

ErrorCode mem_acquire_on_arena(
    i32* offset,
    ClArena*  arena,
    const i32 sz) {
    ASSERT(
        arena->cap >= arena->sz + sz,
        WARN,
        "Arena can't allocate requested sz") {
        return MEMALF;
    }
    ASSERT(arena, WARN, "NULL as ptr to arena passed") {
        return CONTRV;
    }
    if (offset) {
        *offset = arena->sz;
    }
    arena->sz += sz;
    return OK;
}

// TODO: Fix nodes deallocation
ErrorCode mem_release_arena(ClArena* arena) {
    ASSERT(arena, WARN, "NULL as ptr to arena passed") {
        return CONTRV;
    }
    free(arena->begin);
    arena->begin = NULL;
    arena->sz    = 0;
    arena->cap   = 0;
    return OK;
}

ErrorCode mem_realloc_arena(
    ClArena*  arena,
    const i32 new_cap) {
    char* realloced;
    ASSERT(arena, WARN, "NULL as ptr to arena passed") {
        return CONTRV;
    }
    realloced = realloc(arena->begin, new_cap);
    ASSERT(realloced, WARN, "Realloc failed") {}
    arena->begin = realloced;
    arena->cap   = new_cap;
    return OK;
}
