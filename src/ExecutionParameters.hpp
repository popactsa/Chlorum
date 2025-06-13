#ifndef EXECUTION_PARAMETERS_HPP
#define EXECUTION_PARAMETERS_HPP

#include <string>

struct ExecutionParameters {
    ExecutionParameters(int argc, const char* argv[]) noexcept;
    std::string exec_file;
    std::string input_file = "input.txt";
    std::string output_file;
    std::string err_file;
};

#endif  // EXECUTION_PARAMETERS_HPP
