#ifndef SERVER_HPP
#define SERVER_HPP

#include "Socket.hpp"
#include "TcpSocket.hpp"

class Server {
public:
    static constexpr std::uint32_t qDefaultMaxConn = SOMAXCONN;
    Server() = default;
    Server(const dash::SocketAddrIn& addr);
    void start(const dash::SocketAddrIn& addr, int max_conn = qDefaultMaxConn);
    void start(int max_conn = qDefaultMaxConn);

private:
    void accept_clients();
    dash::TcpListener lsock_;
};

#endif  // SERVER_HPP
