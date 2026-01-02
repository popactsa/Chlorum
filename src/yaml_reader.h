#ifndef YAML_READER_H
#define YAML_READER_H

#include <stdio.h>
#include "error_handling.h"
#include "type_aliases.h"

typedef enum ClYamlEvent_e {
    CL_YAML_EVENT_UP,
    CL_YAML_EVENT_DOWN,
    CL_YAML_EVENT_COMMA,
    CL_YAML_EVENT_PROP_VALUE_ARRAY_BEGIN,
    CL_YAML_EVENT_PROP_VALUE_ARRAY_END,
    CL_YAML_EVENT_PROP_NAME,
    CL_YAML_EVENT_PROP_VALUE,
    CL_YAML_EVENT_EOF
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
        CL_YAML_STATE_VALUE_ARRAY,
        CL_YAML_STATE_PROP,
        CL_YAML_STATE_PARSE_ERROR,
        CL_YAML_STATE_ERROR,
        CL_YAML_STATE_EOF
    } state;

    ClYamlVariable* hierarchy;
} ClYamlReader;

ErrorCode ClYamlReaderInit(
    ClYamlReader* reader,
    const char*   filename);
ErrorCode ClYamlReaderNext(
    const ClYamlVariable* variable,
    ClYamlReader*         reader);

#endif /* YAML_READER_H */
