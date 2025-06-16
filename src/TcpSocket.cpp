#include "TcpSocket.hpp"

#include "error_handling.hpp"

////////////////////// TcpConnection //////////////////////

TcpConnection::TcpConnection(int fd) noexcept : TcpSocket(fd) {};

void TcpConnection::connect(const SocketAddrIn& addr) {
    auto [c_addr, size] = addr.data();
    if (::connect(fd_, &c_addr, size) == -1) {
        throw dash::SocketException("Can't establish connection\n");
    }
}

std::size_t TcpConnection::send(const void* data, std::size_t len) {
    ssize_t sent = ::send(fd_, data, len, 0);
    if (sent == -1) {
        throw dash::SocketException("Can't send data\n");
    }
    return sent;
}

std::size_t TcpConnection::recv(void* buf, std::size_t len) {
    ssize_t received = ::recv(fd_, buf, len, 0);
    if (received == -1) {
        throw dash::SocketException("Can't receive data\n");
    }
    return received;
}

////////////////////// TcpListener //////////////////////

void TcpListener::listen(int max_conn) {
    if (::listen(fd_, max_conn) == -1) {
        throw dash::SocketException("Can't start listen\n");
    }
}

TcpConnection TcpListener::accept(SocketAddrIn* addr_ptr) {
    int new_fd{0};
    if (addr_ptr == nullptr) {
        new_fd = ::accept(fd_, nullptr, nullptr);
    } else {
        new_fd = ::accept(fd_, &addr_ptr->addr_, &addr_ptr->size_);
    }
    if (new_fd == -1) {
        throw dash::SocketException("Can't accept a new connection\n");
    }
    return {new_fd};
}

void TcpListener::start(const SocketAddrIn& addr, int max_conn) {
    bind(addr);
    listen(max_conn);
}

TcpConnection TcpListener::accept_client(SocketAddrIn* addr_ptr) {
    return accept(addr_ptr);
}
