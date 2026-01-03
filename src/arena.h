#ifndef ARENA_H
#define ARENA_H

#include "error_handling.h"
#include "type_aliases.h"

/* ClArena is automatically freed at program exit chunk of memory
 * It is allocated with malloc and stored in ClArenaNode(single-linked list)
 * You should store a pointer to arena
 *
 * construct_arena saves a pointer to allocated arena with given capacity
 *
 * destruct_arenas cleans all arenas in single-linked list stored in
 * arena_nodes_head. Called at program exit
 *
 * mem_acquire_on_arena is not necessary to use arena, but it can be useful to
 * verify, that there is enough memory on arena left unoccupied with meaningful
 * data
 *
 * mem_release_arena just wipes out arena completely freeing all allocated
 * memory
 *
 * mem_realloc_arena is just a realloc of allocated for arena memory. It can be
 * used is pair with mem_release_arena to reset arena
 * */

typedef struct ClArena_s {
    char* begin;
    i32   sz;
    i32   cap;
} ClArena;

typedef struct ClArenaNode_s {
    ClArena               arena;
    struct ClArenaNode_s* next;
} ClArenaNode;

extern ClArenaNode* arena_nodes_head;
extern i32          arena_nodes_cnt;

ErrorCode construct_arena(
    ClArena** result,
    const i32 cap);

void destruct_arenas(void);

ErrorCode mem_acquire_on_arena(
    void**    result,
    ClArena*  arena,
    const i32 sz);

ErrorCode mem_release_arena(ClArena* arena);

ErrorCode mem_realloc_arena(
    ClArena*  arena,
    const i32 new_cap);

#endif /* ARENA_H */
