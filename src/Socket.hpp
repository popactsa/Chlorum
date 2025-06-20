#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <type_traits>
#include <utility>

#include "auxiliary_functions.hpp"
#include "error_handling.hpp"

namespace dash {

///////////////////////// Concepts /////////////////////////
template<typename SA>
concept SockAddrIn
    = std::is_same_v<SA, sockaddr_in> || std::is_same_v<SA, sockaddr_in6>;

///////////////////////// Structures /////////////////////////
// This struct can be used to pass addr without any explicit conversion as a
// const reference
struct SocketAddrIn {
    template<typename SA>
        requires dash::SockAddrIn<SA>
    SocketAddrIn(const SA& addr) : size_(sizeof(addr)) {
        std::memcpy(&addr_, &addr, sizeof(addr));
    }
    std::pair<const sockaddr&, socklen_t> data() const noexcept {
        return {addr_, size_};
    }
    sockaddr addr_;
    socklen_t size_;
};

class Socket {
public:
    enum class Status : char {
        qOpen = 1,
        qBound = 1 << 1,
        qListening = 1 << 2,
        qConnected = 1 << 3,
        qReusable = 1 << 4,
        qStarted = qBound + qListening
    };
    Socket(int domain, int type, int protocol);
    void bind(const SocketAddrIn& addr);
    int fd() const noexcept;
    operator int() const noexcept;
    bool close() noexcept;
    void set_nb();
    inline dash::Flag<Status> status_flags() const noexcept {
        return status_flags_;
    }
    ~Socket() noexcept;

protected:
    Socket(int fd) noexcept;
    int fd_;
    dash::Flag<Status> status_flags_;
};
///////////////////////// Exceptions /////////////////////////
// A common class for exceptions thrown when checking parameters validity

class SocketException : public std::exception {
protected:
    std::string msg_;

public:
    explicit SocketException(const std::string& msg) : msg_(msg) {}
    const char* what() const noexcept override {
        return msg_.c_str();
    }
};

class IncorrectSocketStatus : public SocketException {
public:
    explicit IncorrectSocketStatus(const std::string& msg) :
        SocketException(msg) {}
};

class SocketCreationError : public SocketException {
public:
    explicit SocketCreationError(const std::string& msg) :
        SocketException(msg) {}
    explicit SocketCreationError() :
        SocketCreationError("Can't create a socket") {}
};

class ConnectionEOF : public SocketException {
public:
    explicit ConnectionEOF(const std::string& msg) : SocketException(msg) {}
    explicit ConnectionEOF() : ConnectionEOF("EOF/Connection closed") {}
};

}  // namespace dash

#endif  // SOCKET_HPP
