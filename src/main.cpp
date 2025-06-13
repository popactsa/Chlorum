#include <fstream>

#include "ExecutionParameters.hpp"

int main(int argc, const char* argv[]) {
    ExecutionParameters args(argc, argv);
    std::ifstream fin(args.input_file);
    return 0;
}
