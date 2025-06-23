#ifndef TCP_LISTENER_HPP
#define TCP_LISTENER_HPP

#include "TcpSocket.hpp"

namespace dash {
template<typename Packet_t>
    requires PacketFormat<Packet_t>
class TcpConnection;

template<typename Packet_t>
    requires PacketFormat<Packet_t>
class TcpListener : public TcpSocket {
public:
    TcpListener() = default;
    void        listen(int max_conn = qDefaultMaxConn);
    inline void start(const SocketAddrIn& addr, int max_conn = qDefaultMaxConn);
    // TcpConnection<Packet_t> accept(SocketAddrIn* addr_ptr = nullptr);
    std::unique_ptr<TcpConnection<Packet_t>> accept_on_heap(
        SocketAddrIn* addr_ptr = nullptr) noexcept;
};

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpListener<Packet_t>::start(const SocketAddrIn& addr, int max_conn) {
    bind(addr);
    listen(max_conn);
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpListener<Packet_t>::listen(int max_conn) {
    set_nb();
    if (::listen(fd_, max_conn) == -1) {
        throw dash::SocketException("Can't start listen\n");
    }
    status_flags_ |= Status::qListening;
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
std::unique_ptr<TcpConnection<Packet_t>> TcpListener<Packet_t>::accept_on_heap(
    SocketAddrIn* addr_ptr) noexcept {
    if (!(status_flags_ & Status::qListening)) {
        std::cerr << "Can't accept, not listening" << std::endl;
        return nullptr;
    } else if (!(status_flags_ & Status::qBound)) {
        std::cerr << "Can't accept, not bound" << std::endl;
        return nullptr;
    }
    int new_fd{0};
    if (addr_ptr == nullptr) {
        new_fd = ::accept(fd_, nullptr, nullptr);
    } else {
        new_fd = ::accept(fd_, &addr_ptr->addr_, &addr_ptr->size_);
    }
    if (new_fd == -1) {
        return nullptr;
    }
    return std::unique_ptr<TcpConnection<Packet_t>>(
        new TcpConnection<Packet_t>(new_fd));
}

}  // namespace dash

#endif  // TCP_LISTENER_HPP
