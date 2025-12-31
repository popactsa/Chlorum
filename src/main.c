#include <stdlib.h>
#define LOGGING 0

#include "arena.h"
#include "error_handling.h"

int main(void) {
    ErrorCode_t ec;

    ec = MEMBND;
    CHECK(ec, INFO, "hi");
    CHECK(ec, WARN, NULL);

    int error = 5;
    CHECK_ANY(error, INFO, "hello", 2, 3, 4);
    CHECK_ANY(error, WARN, "hello", 1, 5);
    CHECK_ANY(error, WARN, "hello");

    Arena_t* arena;
    if (CHECK(construct_arena(&arena, 3), ERROR, "Failed arena allocation")) {
        exit(EXIT_FAILURE);
    }
    return 0;
}
