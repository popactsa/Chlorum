#ifndef AUXILIARY_FUNCTIONS_H
#define AUXILIARY_FUNCTIONS_H

#include <algorithm>
#include <bitset>
#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>

namespace dash {

struct RcFreePrint {
    mutable std::mutex mtx;
    void               operator()(std::string_view msg) const {
        const std::lock_guard<std::mutex> lock(mtx);
        std::cout << msg << std::flush;
    }
};

constexpr RcFreePrint rc_free_print;

template<typename F>
struct FinalAction {
    explicit FinalAction(F f) noexcept : act(f) {}
    ~FinalAction() noexcept {
        act();
    }
    F act;
};

template<typename F>
[[nodiscard]] inline auto Finally(F f) noexcept {
    return FinalAction{f};
}

template<typename T>
struct Type {
    static constexpr char dummy_ = 0;
};

template<typename T>
inline constexpr const void* qID = &Type<T>::dummy_;

template<typename T>
inline static std::size_t ID() noexcept {
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

template<typename E>
    requires std::is_enum_v<E>
class Flag {
public:
    Flag(E rhs) noexcept : bits_(static_cast<std::underlying_type_t<E>>(rhs)) {}
    Flag()                       = default;
    Flag(const Flag&)            = default;
    Flag& operator=(const Flag&) = default;

    void reset() noexcept {
        bits_.reset();
    }
    Flag& operator|=(const Flag& rhs) noexcept {
        bits_ |= rhs.bits_;
        return *this;
    }
    [[nodiscard]]
    Flag operator|(const Flag& rhs) const noexcept {
        Flag result = *this;
        result |= rhs;
        return result;
    }
    Flag& operator&=(const Flag& rhs) noexcept {
        bits_ &= rhs.bits_;
        return *this;
    }
    [[nodiscard]]
    Flag operator&(const Flag& rhs) const noexcept {
        Flag result = *this;
        result &= rhs;
        return result;
    }

    // Disable a certain flag
    Flag& operator*=(const Flag& rhs) noexcept {
        bits_ &= ~rhs.bits_;
        return *this;
    }

    Flag& operator~() noexcept {
        ~bits_;
        return *this;
    }

    [[nodiscard]]
    bool operator==(Flag rhs) const noexcept {
        return bits_ == rhs.bits_;
    }

    [[nodiscard]]
    bool operator!=(Flag rhs) const noexcept {
        return bits_ != rhs.bits_;
    }
    [[nodiscard]]
    bool any() const noexcept {
        return bits_.any();
    }
    [[nodiscard]] explicit operator bool() const noexcept {
        return any();
    }
    operator std::bitset<sizeof(E) * CHAR_BIT>() const noexcept {
        return bits_;
    }

private:
    std::bitset<sizeof(E) * CHAR_BIT> bits_;
};

constexpr std::vector<std::string> SplitString(std::string_view init,
                                               const char       sep) noexcept {
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

#endif  // AUXILIARY_FUNCTIONS_H
