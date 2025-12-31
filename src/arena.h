#ifndef ARENA_H
#define ARENA_H

#include "error_handling.h"
#include "type_aliases.h"

typedef struct Arena {
    void* begin;
    void* end;
} Arena_t;

#define MAXARENAS 1
extern Arena_t* allocated_arenas[MAXARENAS];

ErrorCode_t construct_arena(
    Arena_t** result,
    const i32 sz);
void destruct_arenas(void);

#endif /* ARENA_H */
