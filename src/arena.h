#ifndef ARENA_H
#define ARENA_H

#include "error_handling.h"
#include "type_aliases.h"

typedef struct Arena_s {
    char* begin;
    i32   sz;
    i32   cap;
} Arena;

typedef struct ArenaNode_s {
    Arena               arena;
    struct ArenaNode_s* next;
} ArenaNode;

extern ArenaNode* arena_nodes_head;
extern i32        arena_nodes_cnt;

ErrorCode construct_arena(
    Arena**   result,
    const i32 cap);

void destruct_arenas(void);

ErrorCode mem_acquire_on_arena(
    void**    result,
    Arena*    arena,
    const i32 sz);

ErrorCode mem_release_arena(Arena* arena);

ErrorCode mem_realloc_arena(
    Arena*    arena,
    const i32 new_cap);

#endif /* ARENA_H */
