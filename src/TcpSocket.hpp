#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

extern "C" {
#include <netinet/in.h>
#include <sys/socket.h>
}

#include "Protocol.hpp"
#include "Socket.hpp"

namespace dash {
class TcpSocket : public Socket {
public:
    static constexpr int qDefaultMaxConn = SOMAXCONN;
    TcpSocket() : Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) {}

protected:
    TcpSocket(int fd) noexcept : Socket{fd} {}
};

// limit to just a single packet format
// change in future to be less specific
template<typename Packet_t>
concept PacketFormat = std::is_same_v<Packet_t, dash::proto::Packet>;

}  // namespace dash

#endif  // TCP_SOCKET_HPP
