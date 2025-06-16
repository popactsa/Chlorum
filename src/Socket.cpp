#include "Socket.hpp"

#include <asm-generic/socket.h>

Socket::Socket(int domain, int type, int protocol) :
    fd_{::socket(domain, type, protocol)} {
    if (errno) {
        throw dash::SocketCreationError();
    }
}

void Socket::bind(const SocketAddrIn& addr) {
    if (fd_ == -1) {
        throw dash::SocketException("Invalid fd\n");
    }
    const auto& [c_addr, size] = addr.data();
    if (::bind(fd_, &c_addr, size) == -1) {
        throw dash::SocketException("Fail on binding\n");
    }
    int reuse = 1;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))
        == -1) {
        std::cerr << "Can't set socket to reusable" << std::endl;
        ;
    };
}

int Socket::fd() const noexcept {
    return fd_;
}

Socket::operator int() const noexcept {
    return fd();
}

bool Socket::close() const noexcept {
    return ::close(fd_) == -1 ? false : true;
}

Socket::~Socket() noexcept {
    close();
}
