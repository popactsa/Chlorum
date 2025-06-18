#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#include "error_handling.hpp"
extern "C" {
#include <netinet/in.h>
#include <sys/socket.h>
}
#include <cstdint>

#include "Socket.hpp"

namespace dash {
class TcpSocket : public Socket {
public:
    static constexpr int qDefaultMaxConn = SOMAXCONN;
    TcpSocket() : Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) {}

protected:
    TcpSocket(int fd) noexcept : Socket{fd} {}
};

class TcpListener;

class TcpConnection : public TcpSocket {
public:
    TcpConnection() = default;
    void connect(const SocketAddrIn& addr);
    std::uint32_t send(const void* data, std::uint32_t len);
    std::uint32_t recv(void* buf, std::uint32_t len);
    void write_all(const char* rbuf, std::size_t sz);
    void read_all(char* buf, std::size_t sz);

protected:
    friend TcpListener;
    TcpConnection(int fd) noexcept;
};

class TcpListener : public TcpSocket {
public:
    TcpListener() = default;
    TcpConnection accept(SocketAddrIn* addr_ptr);
    void listen(int max_conn = qDefaultMaxConn);
    void start(const SocketAddrIn& addr, int max_conn = qDefaultMaxConn);
    TcpConnection accept_client(SocketAddrIn* addr_ptr = nullptr);
};
}  // namespace dash

#endif  // TCP_SOCKET_HPP
