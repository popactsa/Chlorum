#include "TcpSocket.hpp"

#include "error_handling.hpp"

namespace dash {
////////////////////// TcpConnection //////////////////////

TcpConnection::TcpConnection(int fd) noexcept : TcpSocket(fd) {};

void TcpConnection::connect(const SocketAddrIn& addr) {
    auto [c_addr, size] = addr.data();
    if (::connect(fd_, &c_addr, size) == -1) {
        throw dash::SocketException("Can't establish connection\n");
    } else {
        status_flags_ |= Status::qConnected;
    }
}

std::uint32_t TcpConnection::send(const void* data, std::uint32_t len) {
    ssize_t sent = ::send(fd_, data, len, 0);
    if (sent == -1) {
        throw dash::SocketException("Can't send data\n");
    }
    return sent;
}

std::uint32_t TcpConnection::recv(void* buf, std::uint32_t len) {
    ssize_t received = ::recv(fd_, buf, len, 0);
    if (received == -1) {
        throw dash::SocketException("Can't receive data\n");
    }
    return received;
}

void TcpConnection::write_all(const char* rbuf, std::size_t sz) {
    while (sz > 0) {
        ssize_t rv = ::write(*this, rbuf, sz);
        if (rv < 0) {
            throw dash::SocketException("Can't write to socket\n");
        } else if (rv == 0) {
            throw dash::ConnectionEOF("Can't write to socket: EOF/Closed\n");
        }
#ifdef DASH_DEBUG
        std::cout << std::format("WRITE : {} of {}\n", rv, sz);
#endif  // DASH_DEBUG
        sz -= rv;
        rbuf += rv;
    }
}

void TcpConnection::read_all(char* buf, std::size_t sz) {
    while (sz > 0) {
        ssize_t rv = ::read(*this, buf, sz);
        if (rv < 0) {
            throw dash::SocketException("Can't read from socket\n");
        } else if (rv == 0) {
            throw dash::ConnectionEOF("Can't read from socket: EOF/Closed\n");
        }
#ifdef DASH_DEBUG
        std::cout << std::format("READ : {} of {}\n", rv, sz);
#endif  // DASH_DEBUG
        sz -= rv;
        buf += rv;
    }
}

////////////////////// TcpListener //////////////////////

void TcpListener::listen(int max_conn) {
    if (::listen(fd_, max_conn) == -1) {
        throw dash::SocketException("Can't start listen\n");
    } else {
        status_flags_ |= Status::qListening;
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
}  // namespace dash
