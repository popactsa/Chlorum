#include "Client.hpp"

#include "Socket.hpp"

Client::Client(const dash::SocketAddrIn& addr) {
    csock_.connect(addr);
}

void Client::connect(const dash::SocketAddrIn& addr) {
    if (!(csock_.status_flags() & dash::Socket::Status::qConnected)) {
        csock_.connect(addr);
    }
}

void Client::send_packet(const dash::proto::Packet& packet) {
    dash::proto::send_packet(csock_, packet);
}
