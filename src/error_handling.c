#include "error_handling.h"
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "iomanip.h"

const i32  kMaxStrSz = 256;
ErrorLevel gLogging  = INFO;
ErrorLevel gAsserts  = INFO;

static const char* ectos(ErrorCode ec) {
    switch (ec) {
    case OK:
        return "OK";
    case MEMALF:
        return "MEMALF(Memory allocation fail)";
    case MEMBND:
        return "MEMBND(Allocated memory boundaries violation)";
    case MLOGIC:
        return "MLOGIC(Math logic error)";
    case CONTRV:
        return "CONTRV(Contract violation)";
    default:
        return "UNKNOWN";
    }
}

static const char* eltocs(ErrorLevel ec) {
    switch (ec) {
    case INFO:
        return ANSI_BLUE "[INFO]";
    case WARN:
        return ANSI_YELLOW "[WARN]";
    case ERROR:
    case EXIT:
        return ANSI_RED "[ERROR]";
    default:
        return ANSI_GREEN "[UNKNOWN]";
    }
}

void printf_el(
    ErrorLevel  elvl,
    const char* fmt,
    int         line,
    const char* file,
    ...) {
    const char* el_desc;
    va_list     fmt_args;

    va_start(fmt_args);
    el_desc = eltocs(elvl);
    printf(
        "%1$s Logging at l:%2$d [%3$s]:\n"
        "%1$s ",
        el_desc,
        line,
        file);
    vprintf(fmt, fmt_args);
    printf(ANSI_RESET);
}

bool leveled_assert(
    bool        boolean,
    ErrorLevel  elvl,
    const char* fmt,
    int         line,
    const char* file,
    ...) {
    if (gAsserts > elvl) {
        return false;
    }
    if (!boolean) {
        if (gLogging <= elvl) {
            va_list args;
            va_start(args);
            printf_el(elvl, fmt, line, file, args);
        }
        if (elvl == EXIT) {
            exit(EXIT_FAILURE);
        }
    }
    return boolean;
}

ErrorCode check(
    ErrorCode   ec,
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file) {
    if (gLogging > elvl) {
        return ec;
    }
    if (ec == OK) {
        return ec;
    }
    print_log(ec, elvl, msg, line, file);
    if (elvl == EXIT) {
        exit(EXIT_FAILURE);
    }
    return ec;
}

int check_errno(
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file) {
    int saved_errno;
    saved_errno = errno;
    errno = 0;
    if (gLogging > elvl) {
        return saved_errno;
    }
    if (saved_errno == 0) {
        return saved_errno;
    }
    print_log_errno_snapshot(saved_errno, elvl, msg, line, file);
    if (elvl == EXIT) {
        exit(EXIT_FAILURE);
    }
    return saved_errno;
}

int check_errno_omit(
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file,
    ...) {
    va_list omit;
    int     saved_errno, omit_cnt;
    saved_errno = errno;
    errno = 0;
    if (gLogging > elvl) {
        return saved_errno;
    }
    if (saved_errno == 0) {
        return saved_errno;
    }
    va_start(omit);
    omit_cnt = va_arg(omit, int);
    for (int i = 0; i < omit_cnt; ++i) {
        if (saved_errno == va_arg(omit, int)) {
            return saved_errno;
        }
    }
    print_log_errno_snapshot(saved_errno, elvl, msg, line, file);
    if (elvl == EXIT) {
        exit(EXIT_FAILURE);
    }
    return saved_errno;
}

void print_log(
    ErrorCode   ec,
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file) {
    const char *ec_desc, *el_desc;

    ec_desc = ectos(ec);
    el_desc = eltocs(elvl);
    if (msg) {
        fprintf(
            stderr,
            "%1$s Error %4$s occured at l:%2$d [%3$s]\n"
            "%1$s Description: %5$s\n" ANSI_RESET,
            el_desc,
            line,
            file,
            ec_desc,
            msg);
    } else {
        fprintf(
            stderr,
            "%1$s Error %4$s occured at l:%2$d [%3$s]\n" ANSI_RESET,
            el_desc,
            line,
            file,
            ec_desc);
    }
}

void print_log_errno_snapshot(
    int         errno_snapshot,
    ErrorLevel  elvl,
    const char* msg,
    int         line,
    const char* file) {
    const char* el_desc;

    el_desc = eltocs(elvl);
    if (msg) {
        fprintf(
            stderr,
            "%1$s Error %4$s occured at l:%2$d [%3$s]\n"
            "%1$s Description: %5$s\n" ANSI_RESET,
            el_desc,
            line,
            file,
            strerror(errno_snapshot),
            msg);
    } else {
        fprintf(
            stderr,
            "%1$s Error %4$s occured at l:%2$d [%3$s]\n" ANSI_RESET,
            el_desc,
            line,
            file,
            strerror(errno_snapshot));
    }
}
