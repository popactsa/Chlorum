#include <errno.h>
#include <stdio.h>
#include "error_handling.h"
#include "yaml_reader.h"

int main(
    [[maybe_unused]] int    argc,
    [[maybe_unused]] char** argv) {
    ClYamlReader reader;
    ClYamlReaderInit(&reader, "asdf.yaml");
    ClYamlReaderFinish(&reader);
    return 0;
}
