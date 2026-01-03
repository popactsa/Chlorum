#include "yaml_reader.h"
#include "arena.h"
#include "error_handling.h"

ErrorCode ClYamlReaderInit(
    ClYamlReader* reader,
    const char*   filename) {
    FILE*  fin;
    ClArena* arena;
    fin = fopen(filename, "r");
    ASSERT(fin, WARN, "Can't open file") {
        return SYSCF;
    }
    CHECK_S(
        construct_arena(
            &arena, CL_YAML_HIERARCHY_MAX_SIZE * sizeof(ClYamlVariable)),
        WARN,
        NULL,
        {
            fclose(fin);
            return ec;
        });
    *reader = (ClYamlReader){
        .state = CL_YAML_STATE_INIT, .file = fin, .hierarchy = arena};
    return OK;
}

ErrorCode ClYamlReaderFinish(ClYamlReader* reader) {
    ASSERT(
        reader->state != CL_YAML_STATE_ERROR
            && reader->state != CL_YAML_STATE_NULL,
        WARN,
        "Can't finish ClYamlReader at given state") {return CONTRV;}
    fclose(reader->file);
    return OK;
}
