#include "Socket.hpp"

extern "C" {
#include <asm-generic/socket.h>
}
namespace dash {
Socket::Socket(int domain, int type, int protocol) :
    fd_{::socket(domain, type, protocol)} {
    if (fd_ == -1) {
        throw dash::SocketCreationError();
    } else {
        status_flags_ |= Status::qOpen;
    }
}

void Socket::bind(const SocketAddrIn& addr) {
    if (fd_ == -1) {
        throw dash::SocketException("Invalid fd\n");
    }
    const auto& [c_addr, size] = addr.data();
    if (::bind(fd_, &c_addr, size) == -1) {
        throw dash::SocketException("Fail on binding\n");
    } else {
        status_flags_ |= Status::qBound;
    }
    int reuse = 1;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))
        == -1) {
        std::cerr << "Can't set socket to reusable" << std::endl;
        ;
    } else {
        status_flags_ |= Status::qReusable;
    };
}

int Socket::fd() const noexcept {
    return fd_;
}

Socket::operator int() const noexcept {
    return fd();
}

bool Socket::close() noexcept {
    bool result = ::close(fd_) == -1 ? false : true;
    if (result) {
        fd_ = -1;
        status_flags_ *= Status::qOpen;
        return true;
    } else {
        return false;
    }
}

Socket::~Socket() noexcept {
    close();
}
}  // namespace dash
