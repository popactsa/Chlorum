#ifndef ERROR_HANDLING2_H
#define ERROR_HANDLING2_H

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
    PRE
} ErrorCode;

typedef struct Error_s {
    ErrorLevel  lvl;
    ErrorCode   ec;
    const char* desc;
} Error;

#define LOG_ERROR(error) \
    { log_error(error, __LINE__, __FILE__); }
void log_error(
    const Error error,
    int         line,
    const char* file);

#endif /* ERROR_HANDLING2_H */
