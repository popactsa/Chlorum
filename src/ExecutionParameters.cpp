#include "ExecutionParameters.hpp"

#include <unordered_map>

ExecutionParameters::ExecutionParameters(int argc,
                                         const char* argv[]) noexcept {
    std::unordered_map<const char*, std::string&> flag_map{{"-i", input_file},
                                                           {"-o", output_file},
                                                           {"-e", err_file},
                                                           {"-ie", input_file}};
    exec_file = argv[0];
    auto current_flag = flag_map.end();
    for (int i{1}; i < argc; ++i) {
        auto found = flag_map.find(argv[i]);
        if (found != flag_map.end()) {
            current_flag = found;
            continue;
        }
        if (current_flag != flag_map.end()) {
            current_flag->second = argv[i];
            current_flag = flag_map.end();
        }
    }
    if (!output_file.empty() && err_file.empty()) {
        err_file = output_file;
    }
}
