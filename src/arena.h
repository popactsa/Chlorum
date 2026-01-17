#ifndef ARENA_H
#define ARENA_H

#include "error_handling.h"
#include "type_aliases.h"

typedef struct Arena_s {
    char* begin;
    i32   sz;
    i32   cap;
} Arena;

/* Arbitrary arena manipulation functions */

Error construct_arena(
    Arena** arena,
    i32     cap);

void destruct_arena(Arena* arena);

Error realloc_arena(
    Arena* arena,
    i32    new_cap);

#define mem_acquire_arena(acquired, sz) \
    _Generic(                           \
        (acquired),                     \
        i32 * *: mem_acquire_arena_i32, \
        f64 * *: mem_acquire_arena_f64, \
        default: mem_acquire_arena_void)(acquired, sz)

Error mem_acquire_arena_void(
    void** acquired,
    Arena* arena,
    i32    sz);

static inline Error mem_acquire_arena_i32(
    i32**  acquired,
    Arena* arena,
    i32    sz) {
    return mem_acquire_arena_void((void**)acquired, arena, sizeof(i32) * sz);
}

static inline Error mem_acquire_arena_f64(
    f64**  acquired,
    Arena* arena,
    i32    sz) {
    return mem_acquire_arena_void((void**)acquired, arena, sizeof(f64) * sz);
}

Error mem_release_arena(Arena* arena);

/* Default arena manipulation functions */

#define mem_acquire_dft_arena(acquired, sz) \
    _Generic(                               \
        (acquired),                         \
        i32 * *: mem_acquire_dft_arena_i32, \
        f64 * *: mem_acquire_dft_arena_f64, \
        default: mem_acquire_dft_arena_void)(acquired, sz)

Error mem_acquire_dft_arena_void(
    void** acquired,
    i32    sz);

static inline Error mem_acquire_dft_arena_i32(
    i32** acquired,
    i32   sz) {
    return mem_acquire_dft_arena_void((void**)acquired, sizeof(i32) * sz);
}

static inline Error mem_acquire_dft_arena_f64(
    f64** acquired,
    i32   sz) {
    return mem_acquire_dft_arena_void((void**)acquired, sizeof(f64) * sz);
}

Error realloc_dft_arena(i32 new_cap);

Error mem_release_dft_arena(void);

#endif /* ARENA_H */
