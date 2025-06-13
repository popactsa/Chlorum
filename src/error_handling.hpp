#ifndef ERROR_HANDLING_HPP
#define ERROR_HANDLING_HPP

#include <cassert>
#include <exception>
#include <iostream>
#include <string>
#include <type_traits>

namespace dash {
// A common class for exceptions thrown when checking parameters validity
class ParametersException : public std::exception {
protected:
    std::string msg_;

public:
    explicit ParametersException(const std::string& msg) : msg_(msg) {}
    const char* what() const noexcept override {
        return msg_.c_str();
    }
};
class InvalidParameterValue : public ParametersException {
public:
    explicit InvalidParameterValue(const std::string& msg) :
        ParametersException(msg) {}
};

enum class ErrorAction { qIgnore, qThrowing, qTerminating, qLogging };
inline static constexpr ErrorAction qDefaultErrorAction{ErrorAction::qLogging};

////////Tour of C++ 2022 B.Stroustrup p.49/////////
template<ErrorAction action, typename exc, typename C>
constexpr void Expect(const C& cond,
                      const std::string& msg = "<no message provided>") {
    if constexpr (action == ErrorAction::qThrowing) {
        if (!cond()) {
            if constexpr (std::is_constructible_v<exc, const std::string&>) {
                throw exc(msg);
            } else {
                throw exc();
            }
        }
    }
    if constexpr (action == ErrorAction::qTerminating) {
        if (!cond()) {
            std::cerr << msg << std::endl;
            std::terminate();
        }
    }
    if constexpr (action == ErrorAction::qLogging) {
        if (!cond()) {
            std::cerr << "EXPECT logging: " << msg << std::endl;
        }
    }
    // Error_action::qIgnore --> nothing happens
}
}  // namespace dash

#endif  // ERROR_HANDLING_HPP
