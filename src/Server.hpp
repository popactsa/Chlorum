#ifndef SERVER_HPP
#define SERVER_HPP

#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpSocket.hpp"

class Server {
public:
    using packet_t = dash::proto::Packet;
    using conn_t = dash::TcpConnection<packet_t>;
    using listener_t = dash::TcpListener<packet_t>;
    static constexpr std::uint32_t qDefaultMaxConn = SOMAXCONN;
    Server() = default;
    Server(const dash::SocketAddrIn& addr);
    void start(const dash::SocketAddrIn& addr, int max_conn = qDefaultMaxConn);
    void start(int max_conn = qDefaultMaxConn);

protected:
    listener_t lsock_;
    void event_loop();
    std::unique_ptr<conn_t> handle_accept();
    std::unique_ptr<conn_t> handle_read(std::unique_ptr<conn_t>&& conn);
    std::unique_ptr<conn_t> handle_write(std::unique_ptr<conn_t>&& conn);
};

#endif  // SERVER_HPP
