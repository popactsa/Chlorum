#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include "type_aliases.h"

typedef enum ErrorLevel_e {
    INFO,     // Any helpful logs
    WARN,     // Variables are left unchanged
    ERROR,    // Variables have erroneous state(appropriate flags may be set)
    EXIT      // Can't recover program flow
} ErrorLevel;

typedef enum ErrorCode_e {
    OK,        // OK
    MEMALF,    // Memory allocation fail
    MEMBND,    // Allocated memory boundaries violation
    MLOGIC,    // Math logic error
    CONTRV     // Contract violation
} ErrorCode;

extern const i32  kMaxStrSz;
extern ErrorLevel gLogging;
extern ErrorLevel gAsserts;

#define PRINT_EL(elvl, msg, ...) printf_el(elvl, msg, __LINE__, __FILE__)

// TODO: Add format msg version
#define ASSERT(boolean, elvl, fmt, ...) \
    if (!leveled_assert(                \
            boolean, elvl, fmt, __LINE__, __FILE__ __VA_OPT__(, __VA_ARGS__)))

// TODO: Add format msg version
#define CHECK(ec, elvl, msg) if (!check(ec, elvl, msg, __LINE__, __FILE__))

// TODO: Add format msg version
#define LOG_ERRNO(statement, elvl, msg, ...) \
    statement,                               \
        !check_errno##__VA_OPT__(_omit)(     \
            error, elvl, msg, __LINE__, __FILE__ __VA_OPT__(, __VA_ARGS__))

// TODO: Add format msg version
#define CHECK_ERRNO(statement, elvl, msg, ...) \
    statement,                                 \
        check_errno##__VA_OPT__(_omit)(        \
            error, elvl, msg, __LINE__, __FILE__ __VA_OPT__(, __VA_ARGS__))

void printf_el(
    ErrorLevel  elvl,
    const char* fmt,
    int         line,
    const char* file,
    ...);

bool leveled_assert(
    bool        boolean,
    ErrorLevel  elvl,
    const char* fmt,
    int         line,
    const char* file,
    ...);

void print_log(
    ErrorCode   ec,
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file);

void print_log_errno_snapshot(
    int         error,
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file);

ErrorCode check(
    ErrorCode   ec,
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file);

int check_errno(
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file);

int check_errno_omit(
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file,
    ...);

#endif /* ERROR_HANDLING_H */
