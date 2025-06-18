#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpSocket.hpp"

class Client {
public:
    Client() = default;
    Client(const dash::SocketAddrIn& addr);
    void connect(const dash::SocketAddrIn& addr);
    void send_packet(const dash::proto::Packet& packet);

private:
    dash::TcpConnection csock_;
};

#endif  // CLIENT_HPP
