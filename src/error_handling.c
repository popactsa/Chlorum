#include "error_handling.h"
#include <stdio.h>
#include <stdlib.h>
#include "iomanip.h"

ErrorLevel gMinCheckLevel = INFO;

static inline const char* get_ec_str(const ErrorCode ec) {
    switch (ec) {
    case OK:
        return "OK";
    case UNKNOWN:
        return "UNKNOWN";
    case MALLOC:
        return "MALLOC";
    case PRE:
        return "PRE";
    case UNEXP:
        return "UNEXP";
    default:
        return "<Not provided>";
    }
}

static const char* const error_lvl[] = {"INFO", "WARN", "ERROR", "KILL"};

static const char* const error_lvl_color[] = {
    ANSI_BLUE, ANSI_YELLOW, ANSI_RED, ANSI_RED};

void log_error_if(
    const Error error,
    const i32         line,
    const char* file) {
    if (!error.ec) {
        return;
    }
    if (error.lvl < gMinCheckLevel) {
        return;
    }
    printf(
        "%1$s[%2$s] Error: %3$s at l: %5$d(f: %6$s)\n"
        "%1$s[%2$s] Description: %4$s\n" ANSI_RESET,
        error_lvl_color[error.lvl], error_lvl[error.lvl], get_ec_str(error.ec),
        error.desc ? error.desc : "<Not provided>", line, file);
    if (error.lvl == EXIT) {
        exit(EXIT_FAILURE);
    }
}
