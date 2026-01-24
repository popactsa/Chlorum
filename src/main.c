#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "arena.h"
#include "auxiliary_utilities.h"
#include "error_handling.h"
#include "map.h"

int main(
    [[maybe_unused]] int    argc,
    [[maybe_unused]] char** argv) {
    Error e;
    char* c = malloc(32);
    i32 sz;
    hash  hashed;
    e = to_hash_str(&hashed, "hello_dasha", 123);
    LOG_ERROR_IF(e);
    e = from_hash_str(c, &sz, hashed);
    LOG_ERROR_IF(e);
    printf("%ld\n", hashed);
    printf("Total %d characters: %s\n", sz, c);
    free(c);
    return 0;
}
