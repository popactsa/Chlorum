#include "Socket.hpp"

#include <asm-generic/socket.h>
#include <fcntl.h>
namespace dash {
Socket::Socket(int domain, int type, int protocol) :
    fd_{::socket(domain, type, protocol)} {
    if (fd_ == -1) {
        throw dash::SocketCreationError();
    } else {
        status_flags_ |= Status::qOpen;
    }
    // set_nb();
}

Socket::Socket(int fd) noexcept : fd_{fd} {
    // set_nb();
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
    } else {
        status_flags_ |= Status::qReusable;
    };
}

void Socket::set_nb() {
    if (fd_ == -1) {
        // It is considered as default action on construction, so no throw
        return;
    }
    errno     = 0;
    int flags = ::fcntl(fd_, F_GETFL, 0);
    if (errno) {
        throw dash::SocketException("fcntl error when reading flags");
    }
    flags |= O_NONBLOCK;
    errno = 0;
    ::fcntl(fd_, F_SETFL, flags);
    if (errno) {
        throw dash::SocketException("fcntl error when setting flags");
    }
    status_flags_ |= Socket::Status::qNonBlocking;
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
