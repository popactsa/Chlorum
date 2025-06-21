#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <poll.h>

#include <thread>

#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpSocket.hpp"
#include "auxiliary_functions.hpp"

template<typename Packet_t>
    requires dash::PacketFormat<Packet_t>
class Client {
public:
    Client() = default;
    Client(const dash::SocketAddrIn& addr);
    void connect(const dash::SocketAddrIn& addr);
    void do_something(int arg) noexcept;

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
void Client<Packet_t>::do_something(int arg) noexcept {
    int i = 0;
    int sent = 0;
    int max_msg = 3;
    while (i < max_msg) {
        pollfd pfd{csock_.fd(), 0, 0};

        // Set desired events
        if (sent <= i) {
            pfd.events |= POLLOUT;
        }
        pfd.events |= POLLIN;

        int ready = poll(&pfd, 1, -1);
        if (ready < 0) {
            std::cerr << "Poll error" << std::endl;
            break;
        }
        if (ready == 0) {
            std::cerr << "Timeout" << std::endl;
            continue;
        }

        // Handle write
        if ((pfd.revents & POLLOUT) && i >= sent) {
            std::string msg = std::format("Client message {}: {}", sent, arg);
            dash::proto::Packet packet(msg);
            csock_.write_packet(packet);
            csock_.write_all();
            sent++;
            dash::rc_free_print(
                std::string(std::format("CLIENT SENT: {}\n", msg)));
        }

        // Handle read
        if (pfd.revents & POLLIN) {
            csock_.read_all();
            auto read = csock_.read_packet();
            if (read) {
                std::string msg;
                for (char c : read->rmsg_range()) {
                    msg += c;
                }
                msg = std::format("CLIENT RECV: {}\n", msg);
                dash::rc_free_print(msg);
                i++;
            }
        }

        // Handle errors
        if (pfd.revents & POLLERR) {
            dash::rc_free_print(std::string(std::format("Connection error\n")));
            break;
        }
    }

    // Clean shutdown
    csock_.close();
    dash::rc_free_print(std::string(
        std::format("Client finished after receiving {} messages\n", i)));
}

#endif  // CLIENT_HPP
