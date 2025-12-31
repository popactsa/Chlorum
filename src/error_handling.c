#include "error_handling.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "iomanip.h"

const i32 kMaxArrSz = 256;
const i32 kMaxStrSz = 256;

static const char* ectos(ErrorCode_t ec) {
    switch (ec) {
    case OK:
        return "OK";
    case OTHER:
        return "OTHER";
    case MEMBND:
        return "MEMBND";
    case MLOGIC:
        return "MLOGIC";
    case CONTRV:
        return "CONTRV";
    default:
        return "UNKNOWN";
    }
}

static const char* eltocs(ErrorLevel_t ec) {
    switch (ec) {
    case INFO:
        return ANSI_BLUE "[INFO]";
    case WARN:
        return ANSI_YELLOW "[WARN]";
    case ERROR:
    case KILL:
        return ANSI_RED "[ERROR]";
    default:
        return ANSI_GREEN "[UNKNOWN]";
    }
}

ErrorCode_t check(
    ErrorCode_t  ec,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file) {
    if (LOGGING > elvl) {
        return ec;
    }
    if (ec == OK) {
        return ec;
    }
    print_log(ec, elvl, msg, line, file);
    if (elvl == KILL) {
        exit(EXIT_FAILURE);
    }
    return ec;
}

int check_any(
    int          error,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file) {
    if (LOGGING > elvl) {
        return error;
    }
    if (error == 0) {
        return error;
    }
    print_log_any(error, elvl, msg, line, file);
    if (elvl == KILL) {
        exit(EXIT_FAILURE);
    }
    return error;
}

int check_any_omit(
    int          error,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file,
    ...) {
    va_list omit;
    int     omit_cnt;
    if (LOGGING > elvl) {
        return error;
    }
    if (error == 0) {
        return error;
    }
    va_start(omit);
    omit_cnt = va_arg(omit, int);
    for (int i = 0; i < omit_cnt; ++i) {
        if (error == va_arg(omit, int)) {
            return error;
        }
    }
    print_log_any(error, elvl, msg, line, file);
    if (elvl == KILL) {
        exit(EXIT_FAILURE);
    }
    return error;
}

void print_log(
    ErrorCode_t  ec,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file) {
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

void print_log_any(
    int          error,
    ErrorLevel_t elvl,
    const char*  msg,
    int          line,
    const char*  file) {
    const char* el_desc;

    el_desc = eltocs(elvl);
    if (msg) {
        fprintf(
            stderr,
            "%1$s Error %4$d occured at l:%2$d [%3$s]\n"
            "%1$s Description: %5$s\n" ANSI_RESET,
            el_desc,
            line,
            file,
            error,
            msg);
    } else {
        fprintf(
            stderr,
            "%1$s Error %4$d occured at l:%2$d [%3$s]\n" ANSI_RESET,
            el_desc,
            line,
            file,
            error);
    }
}
