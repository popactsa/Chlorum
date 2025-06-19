#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpSocket.hpp"

template<typename Packet_t>
class Client {
public:
    Client() = default;
    Client(const dash::SocketAddrIn& addr);
    void connect(const dash::SocketAddrIn& addr);
    void send_packet(const dash::proto::Packet& packet);

private:
    dash::TcpConnection<Packet_t> csock_;
};

template<typename Packet_t>
Client<Packet_t>::Client(const dash::SocketAddrIn& addr) {
    csock_.connect(addr);
}

template<typename Packet_t>
void Client<Packet_t>::connect(const dash::SocketAddrIn& addr) {
    if (!(csock_.status_flags() & dash::Socket::Status::qConnected)) {
        csock_.connect(addr);
    }
}

template<typename Packet_t>
void Client<Packet_t>::send_packet(const dash::proto::Packet& packet) {
    std::ranges::for_each(packet.rmsg_range(), [](char c) { std::cout << c; });
    std::cout << std::endl;
    ::write(csock_, packet.rbuf(), packet.msg_sz_ + packet.qHeaderLen);
}

#endif  // CLIENT_HPP
