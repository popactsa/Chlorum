#ifndef AUXILIARY_FUNCTIONS_H
#define AUXILIARY_FUNCTIONS_H

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <climits>
#include <iostream>
#include <utility>
#include <variant>

namespace dash {

template<typename F>
struct FinalAction {
    explicit FinalAction(F f) noexcept : act(f) {}
    ~FinalAction() noexcept {
        act();
    }
    F act;
};

template<typename F>
[[nodiscard]] auto Finally(F f) noexcept {
    return FinalAction{f};
}

template<typename T>
struct Type {
    static constexpr char dummy_ = 0;
};

template<typename T>
inline constexpr const void* qID = &Type<T>::dummy_;

template<typename T>
static std::size_t ID() noexcept {
    return reinterpret_cast<std::size_t>(qID<T>);
}

template<typename TimerT = std::chrono::milliseconds>
    requires std::is_convertible_v<TimerT, std::chrono::milliseconds>
[[nodiscard]] auto SetScopedTimer(std::string_view timer_name) noexcept {
    const auto start_time = std::chrono::system_clock::now();
    return FinalAction{[timer_name, start_time]() {
        const char* quot = "===============";
        std::cout << quot << ' ' << timer_name << ' ' << quot << '\n';
        const auto final_time = std::chrono::system_clock::now() - start_time;
        using namespace std::chrono_literals;
        std::cout << std::chrono::duration_cast<TimerT>(final_time) << " \n";
        std::cout << quot << std::string(timer_name.size() + 2, '=') << quot
                  << std::endl;
    }};
}

constexpr std::vector<std::string> SplitString(std::string_view init,
                                               const char sep) noexcept;
}  // namespace dash

#endif  // AUXILIARY_FUNCTIONS_H
