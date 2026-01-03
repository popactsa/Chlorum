#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include "type_aliases.h"

/* With these enums you can specify level of logging and performed checks
 * Set gChecks, gAsserts to set program-wide level of checks.
 * Note: gChecks, gAsserts are variables, not constants, so you can dynamically
 *  change it
 */
typedef enum ErrorLevel_e {
    INFO,     // Any helpful logs
    WARN,     // Variables are left unchanged
    ERROR,    // Variables have erroneous state(appropriate flags may be set)
    EXIT      // Can't recover program flow
} ErrorLevel;

/* More or less common error codes used as return values in Chlorum functions */
typedef enum ErrorCode_e {
    OK,        // OK
    MEMALF,    // Memory allocation fail
    MEMBND,    // Allocated memory boundaries violation
    MLOGIC,    // Math logic error
    CONTRV,    // Contract violation
    SYSCF      // System call fail
} ErrorCode;

/* Use kMaxStrSz at snprintf as upper bound for string length */
extern const i32  kMaxStrSz;
extern ErrorLevel gChecks;
extern ErrorLevel gAsserts;

// TODO: Add tracing
// TODO: Add format msg version for asserts, check functions and macroses

/* 1 - Errno resets at 'check' will be logged as INFO
 * 0 - No logging
 */
#define LOG_ERRNO_RESET 1

/* Check if boolean value is equal true if elvl is enabled with gAsserts
 * If boolean is equal false, logging with elvl is performed
 *
 * Line and file where called is printed
 * Format-string and arguments for printf can be provided */
#define ASSERT(boolean, elvl, fmt, ...) \
    if (!leveled_assert(                \
            boolean, elvl, fmt, __LINE__, __FILE__ __VA_OPT__(, __VA_ARGS__)))

// TODO: Add format msg version
/* Check if ecode is OK if elvl is enabled with gChecks
 * If ecode is not OK, logging with elvl is performed
 *
 * Message can be provided optionally. If NULL passed, no user message printed
 * Example usage: CHECK((ErrorCode)cool_check(...), WARN, "Hello, something is
 * wrong"){verybad = true;}
 * */
#define CHECK(ecode, elvl, msg) if (check(ecode, elvl, msg, __LINE__, __FILE__))

/* Check if ecode is OK if elvl is enabled with gChecks
 * If ecode is not OK, logging with elvl is performed
 * May use ec in body
 *
 * Line and file where called is printed
 * Message can be provided optionally. If NULL passed, no user message printed
 * Example usage: CHECK_S((ErrorCode)cool_check(...), WARN, "Hello, something is
 * wrong", {verybad = ec;})
 * */
#define CHECK_S(ecode, elvl, msg, body)                   \
    {                                                     \
        ErrorCode ec;                                     \
        ec = check(ecode, elvl, msg, __LINE__, __FILE__); \
        if (ec) {                                         \
            body                                          \
        }                                                 \
    }

/* Check if errno is OK if elvl is enabled with gChecks
 * If errno is not 0, logging with elvl is performed
 *
 * Line and file where called is printed
 * Message can be provided optionally. If NULL passed, no user message printed
 * List of omitted errors can be provided optionally
 * Example usage: CHECK_ERRNO(malloc(VERY_MANY), WARN, "Hello, something is
 * wrong"){verybad = true;}
 * */
#define CHECK_ERRNO(statement, elvl, msg, ...) \
    statement;                                 \
    if (!check_errno##__VA_OPT__(_omit)(       \
            elvl, msg, __LINE__, __FILE__ __VA_OPT__(, __VA_ARGS__)))

void print_log_bool(
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
