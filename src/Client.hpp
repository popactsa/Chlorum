#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <thread>

#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpSocket.hpp"

template<typename Packet_t>
    requires dash::PacketFormat<Packet_t>
class Client {
public:
    Client() = default;
    Client(const dash::SocketAddrIn& addr);
    void connect(const dash::SocketAddrIn& addr);
    void do_something() noexcept;

private:
    dash::TcpConnection<Packet_t> csock_;
};

template<typename Packet_t>
    requires dash::PacketFormat<Packet_t>
Client<Packet_t>::Client(const dash::SocketAddrIn& addr) {
    csock_.connect(addr);
}

template<typename Packet_t>
    requires dash::PacketFormat<Packet_t>
void Client<Packet_t>::connect(const dash::SocketAddrIn& addr) {
    if (!(csock_.status_flags() & dash::Socket::Status::qConnected)) {
        csock_.connect(addr);
    }
}

template<typename Packet_t>
    requires dash::PacketFormat<Packet_t>
void Client<Packet_t>::do_something() noexcept {
    for (int i{0}; i < 3; ++i) {
        std::string msg = std::format("that's what she said : {}", i);
        dash::proto::Packet packet(msg);
        csock_.write_packet(packet);
        csock_.write_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        csock_.read_all();
        auto read = csock_.read_packet();
        if (read) {
            std::cout << "CLIENT RECV : ";
            std::ranges::for_each(read->rmsg_range(),
                                  [](char c) { std::cout << c; });
            std::cout << std::endl;
        }
    }
}

#endif  // CLIENT_HPP
