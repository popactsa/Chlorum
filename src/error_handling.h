#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include "type_aliases.h"

typedef enum ErrorLevel {
    INFO,
    WARN,
    ERROR,
    EXIT
} ErrorLevel_t;

typedef enum ErrorCode {
    OK,        // OK
    OTHER,     // Any unspecified error
    MEMALF,    // Memory allocation fail
    MEMBND,    // Allocated memory boundaries violation
    MLOGIC,    // Math logic error
    CONTRV,    // Contract violation
} ErrorCode_t;

extern const i32    kMaxArrSz;
extern const i32    kMaxStrSz;
extern ErrorLevel_t gLogging;

#define PRINT_EL(elvl, msg, ...) printf_el(elvl, msg, __LINE__, __FILE__)

// TODO: Add format msg version
#define CHECK(ec, elvl, msg) check(ec, elvl, msg, __LINE__, __FILE__)

// TODO: Add format msg version
#define CHECK_ANY(error, elvl, msg, ...) \
    check_any##__VA_OPT__(_omit)(        \
        error, elvl, msg, __LINE__, __FILE__ __VA_OPT__(, __VA_ARGS__))

void printf_el(
    ErrorLevel_t elvl,
    const char*  fmt,
    int          line,
    const char*  file,
    ...);

void print_log(
    ErrorCode_t  ec,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file);

void print_log_any(
    int          error,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file);

ErrorCode_t check(
    ErrorCode_t  ec,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file);

int check_any(
    int          error,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file);

int check_any_omit(
    int          error,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file,
    ...);

#endif /* ERROR_HANDLING_H */
