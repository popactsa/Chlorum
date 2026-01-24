#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include "type_aliases.h"

typedef enum ErrorLevel_e {
    INFO,     // Any helpful logs
    WARN,     // Variables are left unchanged or any fallbacks
    ERROR,    // Variables have erroneous state
    EXIT      // Can't recover program flow
} ErrorLevel;

extern ErrorLevel gMinCheckLevel;

typedef enum ErrorCode_e {
    OK,
    UNKNOWN,
    MALLOC,
    PRE,
    UNEXP
} ErrorCode;

// TODO: Support formatting
typedef struct Error_s {
    ErrorLevel  lvl;
    ErrorCode   ec;
    const char* desc;
} Error;

#define LOG_ERROR_IF(e_) \
    { log_error_if(e_, __LINE__, __FILE__); }
void log_error_if(
    const Error error,
    const i32   line,
    const char* file);

#define RETHROW_IF(e_) \
    {                  \
        if (e_.ec) {   \
            return e_; \
        }              \
    }

#endif /* ERROR_HANDLING_H */
