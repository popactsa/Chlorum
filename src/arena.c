#include "arena.h"
#include <stdlib.h>
#include "error_handling.h"

Arena_node_t* arena_nodes = NULL;
i32 arena_nodes_cnt = 0;

ErrorCode_t construct_arena(
    Arena_t** result,
    const i32 cap) {
    if (CHECK_ANY(
            result == NULL,
            ERROR,
            "Nowhere to return result. NULL received")) {
        return CONTRV;
    }
    Arena_node_t* arena_node = malloc(sizeof(Arena_node_t));
    arena_node->next = arena_nodes;
    Arena_t* arena = &arena_node->arena;
    arena->begin   = malloc(cap);
    if (!arena->begin) {
        return MEMALF;
    }
    arena->cap = cap;
    arena->sz  = 0;

    arena_nodes = arena_node;
    if (arena_nodes_cnt == 0) {
        CHECK_ANY(atexit(destruct_arenas), EXIT, NULL);
    }
    ++arena_nodes_cnt;
    *result = arena;
    return OK;
}

void destruct_arenas(void) {
    Arena_node_t* next;
    while (arena_nodes_cnt) {
        next = arena_nodes->next;
        if (arena_nodes->arena.begin) {
            free(arena_nodes->arena.begin);
        }
        free(arena_nodes);
        arena_nodes = next;
        --arena_nodes_cnt;
    }
}

ErrorCode_t mem_acquire_on_arena(
    void**    result,
    Arena_t*  arena,
    const i32 sz) {
    if (CHECK_ANY(
            arena->cap < arena->sz + sz,
            WARN,
            "Arena can't allocate requested sz")) {
        return MEMALF;
    }
    if (CHECK_ANY(
            !arena,
            WARN,
            "NULL as ptr to arena passed")) {
        return CONTRV;
    }
    *result    = arena->begin + arena->sz;
    arena->sz += sz;
    return OK;
}

//TODO: Fix nodes deallocation
ErrorCode_t mem_release_arena(Arena_t* arena) {
    if (CHECK_ANY(
            !arena,
            WARN,
            "NULL as ptr to arena passed")) {
        return CONTRV;
    }
    free(arena->begin);
    arena->begin = NULL;
    arena->sz = 0;
    arena->cap = 0;
    return OK;
}

ErrorCode_t mem_realloc_arena(Arena_t* arena, const i32 new_cap) {
    if (CHECK_ANY(
            !arena,
            WARN,
            "NULL as ptr to arena passed")) {
        return CONTRV;
    }
    char* realloced = realloc(arena->begin, new_cap);
    CHECK_ANY(!realloced, WARN, "Realloc failed");
    arena->begin = realloced;
    arena->cap = new_cap;
    return OK;
}
