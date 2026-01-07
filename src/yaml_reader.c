#include "yaml_reader.h"
#include <stdlib.h>
#include "arena.h"
#include "error_handling.h"

static constexpr i32 kClYamlDefaultVariablesArenaCap =
    32 * sizeof(ClYamlVariable);
static ClArena* variables_arena = NULL;
static char     rbuf[kMaxStrSz];
static char     wbuf[kMaxStrSz];

#define RCHECK_S(ecode, elvl, msg)           \
    CHECK_S(ecode, elvl, msg, {              \
        reader->state = CL_YAML_STATE_ERROR; \
        return ec;                           \
    })

#define RASSERT(boolean, elvl, msg, ecode_to_throw) \
    ASSERT(boolean, elvl, msg) {                    \
        reader->state = CL_YAML_STATE_ERROR;        \
        return ecode_to_throw;                      \
    }

static bool IsCharEvent(char c) {
    switch (c) {
    case CL_YAML_EVENT_EOF:
    case CL_YAML_EVENT_COMMA:
    case CL_YAML_EVENT_SPACE:
    case CL_YAML_EVENT_ENDL:
        return true;
    default:
        return false;
    }
}

ErrorCode ClYamlReaderInit(
    ClYamlReader* reader,
    const char*   filename) {
    FILE*    fin;
    ClArena* hier;
    fin = fopen(filename, "r");
    ASSERT(fin, WARN, "Can't open file") {
        return SYSCF;
    }
    CHECK_S(
        construct_arena(
            &hier, CL_YAML_HIERARCHY_MAX_SIZE * sizeof(ClYamlVariable)),
        WARN, NULL, {
            fclose(fin);
            return ec;
        });
    reader->hier  = (ClYamlVariable*)hier->begin;
    reader->state = CL_YAML_STATE_INIT;
    reader->file  = fin;
    mem_acquire_on_arena(
        NULL, hier, CL_YAML_HIERARCHY_MAX_SIZE * sizeof(ClYamlVariable));
    reader->hier_depth = 0;
    return OK;
}

ErrorCode ClYamlReaderFinish(ClYamlReader* reader) {
    ASSERT(
        reader->state != CL_YAML_STATE_ERROR
            && reader->state != CL_YAML_STATE_NULL,
        WARN, "Can't finish ClYamlReader at given state") {
        return CONTRV;
    }
    fclose(reader->file);
    reader->state = CL_YAML_STATE_NULL;
    return OK;
}

static ErrorCode ReadUntilNextReaderEvent(
    i32*         rbuf_pos,
    ClYamlEvent* event) {
    char  c;
    char* wbuf_cursor = wbuf;
    while (rbuf[*rbuf_pos]) {
        c = rbuf[*rbuf_pos++];
        if (IsCharEvent(c)) {
            break;
        }
        *wbuf_cursor = c;
        ++wbuf_cursor;
    }
    *wbuf  = '\0';
    *event = c;
    return OK;
}

static ErrorCode MaybeResizeArenaToFitVariable(ClYamlReader* reader) {
    if (!variables_arena) {
        RCHECK_S(
            construct_arena(&variables_arena, kClYamlDefaultVariablesArenaCap),
            ERROR, "Can't construct arena buffer for variables")
    } else if (
        variables_arena->cap
        < variables_arena->sz + (i32)sizeof(ClYamlVariable)) {
        RCHECK_S(
            mem_realloc_arena(
                variables_arena,
                variables_arena->cap * kClArenaDefaultResizeRatio),
            ERROR, "Can't resize arena")
    }
    return OK;
}

static ErrorCode ProcessReaderEvent(
    ClYamlEvent   event,
    ClYamlReader* reader) {
    switch(event) {
        CL_YAML_EVENT_EOF:
            reader->state = CL_YAML_STATE_EOF;
            break;
        CL_YAML_EVENT_COMMA:
            if (reader->state != CL_YAML_STATE_VALUE_ARRAY) {

            }
            break;
    }
    return OK;
}

ErrorCode ClYamlReaderNextLine(
    ClYamlVariable** result,
    const ClYamlVariable*,
    ClYamlReader* reader) {
    i32         result_at_pos;
    ClYamlEvent event;
    ASSERT(result, WARN, "Nowhere to store a result") {
        return CONTRV;
    }
    RASSERT(
        fgets(rbuf, kMaxStrSz, reader->file), ERROR,
        "Can't read line from file", SYSCF);

    for (i32 i = 0;; ++i) {
        ReadUntilNextReaderEvent(&i, &event);
        ProcessReaderEvent(event, &reader);
    }

    MaybeResizeArenaToFitVariable(reader);
    RCHECK_S(
        mem_acquire_on_arena(
            &result_at_pos, variables_arena, sizeof(ClYamlVariable)),
        ERROR, "Can't acquire memory for variable to store")
    *result = (ClYamlVariable*)variables_arena->begin + result_at_pos;
    return OK;
}
