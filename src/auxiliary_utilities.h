#ifndef AUXILIARY_UTILITIES_H
#define AUXILIARY_UTILITIES_H

#include "error_handling.h"
#include "type_aliases.h"

static inline void kill_me(...) {
    LOG_ERROR_IF(((Error){
        .lvl = EXIT, .ec = UNKNOWN, .desc = "Now it's time to debug."}));
}

#define min(lhs, rhs) \
    _Generic((lhs), i32: min_i32, f64: min_f64, default: kill_me)(lhs, rhs)

static inline i32 min_i32(
    const i32 lhs,
    const i32 rhs) {
    return lhs < rhs ? lhs : rhs;
}

static inline f64 min_f64(
    const f64 lhs,
    const f64 rhs) {
    return lhs < rhs ? lhs : rhs;
}

#endif /* AUXILIARY_UTILITIES_H */
