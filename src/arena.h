#ifndef ARENA_H
#define ARENA_H

#include "error_handling.h"
#include "type_aliases.h"

typedef struct Arena {
    char* begin;
    i32   sz;
    i32   cap;
} Arena_t;

typedef struct Arena_node {
    Arena_t            arena;
    struct Arena_node* next;
} Arena_node_t;

extern Arena_node_t* arena_nodes;
extern i32           arena_nodes_cnt;

ErrorCode_t construct_arena(
    Arena_t** result,
    const i32 cap);

void destruct_arenas(void);

ErrorCode_t mem_acquire_on_arena(
    void**    result,
    Arena_t*  arena,
    const i32 sz);

ErrorCode_t mem_release_arena(Arena_t* arena);

ErrorCode_t mem_realloc_arena(
    Arena_t*  arena,
    const i32 new_cap);

#endif /* ARENA_H */
