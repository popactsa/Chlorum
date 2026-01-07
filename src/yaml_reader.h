#ifndef YAML_READER_H
#define YAML_READER_H

#include <stdio.h>
#include "arena.h"
#include "error_handling.h"
#include "type_aliases.h"

typedef enum ClYamlEvent_e {
    CL_YAML_EVENT_EOF   = '\0',
    CL_YAML_EVENT_COMMA = ',',
    CL_YAML_EVENT_SPACE = ' ',
    CL_YAML_EVENT_ENDL  = '\n'
} ClYamlEvent;

typedef struct ClYamlValue_s {
    enum {
        CL_YAML_I32,
        CL_YAML_F64,
        CL_YAML_STR
    } type;

    union {
        i32         I32;
        f64         F64;
        const char* str;
    } value;
} ClYamlValue;

typedef struct ClYamlVariable_s {
    char*                    name;
    struct ClYamlVariable_s* parent;

    union {
        struct ClYamlVariable_s* ancestors;
        ClYamlValue              value;
        ClYamlValue*             values_array;
    };
} ClYamlVariable;

typedef struct ClYamlReader_s {
    FILE* file;

    enum {
        CL_YAML_STATE_NULL,
        CL_YAML_STATE_INIT,
        CL_YAML_STATE_VALUE_ARRAY,
        CL_YAML_STATE_PROP,
        CL_YAML_STATE_PARSE_ERROR,
        CL_YAML_STATE_ERROR,
        CL_YAML_STATE_EOF
    } state;

    ClYamlVariable* hier;
    i32             hier_depth;
} ClYamlReader;

#define CL_YAML_HIERARCHY_MAX_SIZE 5

ErrorCode ClYamlReaderInit(
    ClYamlReader* reader,
    const char*   filename);
ErrorCode ClYamlReaderNextLine(
    ClYamlVariable**      result,
    const ClYamlVariable* prev,
    ClYamlReader*         reader);
ErrorCode ClYamlReaderFinish(ClYamlReader* reader);

#endif /* YAML_READER_H */
