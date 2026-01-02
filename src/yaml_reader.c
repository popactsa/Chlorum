#include "yaml_reader.h"

ErrorCode ClYamlReaderInit(ClYamlReader* reader, const char* filename) {
    reader->file = fopen(filename, "r");
    return OK;
}
