#include "auxiliary_functions.hpp"

namespace dash {
constexpr std::vector<std::string> SplitString(std::string_view init,
                                               const char sep) noexcept {
    std::vector<std::string> result;
    for (auto it = init.cbegin(), prev = it; it != init.cend();) {
        it = std::find(prev, init.cend(), sep);
        if (prev != it) {
            result.emplace_back(prev, it);
        }
        prev = it + 1;
    }
    return result;
}

}  // namespace dash
