#include "Server.hpp"

#include <algorithm>

#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpSocket.hpp"

Server::Server(const dash::SocketAddrIn& addr) {
    lsock_.bind(addr);
}

void Server::start(const dash::SocketAddrIn& addr, int max_conn) {
    if (!(lsock_.status_flags() & dash::Socket::Status::qBound)) {
        lsock_.start(addr, max_conn);
    } else {
        lsock_.listen(max_conn);
    }
    accept_clients();
}

void Server::start(int max_conn) {
    if (!(lsock_.status_flags() & dash::Socket::Status::qBound)) {
        throw dash::SocketException("Trying to start without a binding\n");
    } else {
        lsock_.listen(max_conn);
    }
    accept_clients();
}

void Server::accept_clients() {
    dash::TcpConnection client = lsock_.accept_client();
    dash::proto::Packet packet;
    dash::proto::receive_packet(client, packet);
    std::cout << std::endl;
}
