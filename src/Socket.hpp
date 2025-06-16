#ifndef SOCKET_HPP
#define SOCKET_HPP

extern "C" {
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
}
#include <cerrno>
#include <cstring>
#include <type_traits>
#include <utility>

#include "error_handling.hpp"

namespace dash {
template<typename SA>
concept SockAddrIn
    = std::is_same_v<SA, sockaddr_in> || std::is_same_v<SA, sockaddr_in6>;
}

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
    Socket(int domain, int type, int protocol);
    void bind(const SocketAddrIn& addr);
    int fd() const noexcept;
    operator int() const noexcept;
    bool close() const noexcept;
    ~Socket() noexcept;

protected:
    Socket(int fd) noexcept : fd_{fd} {}
    int fd_;
};

#endif  // SOCKET_HPP
