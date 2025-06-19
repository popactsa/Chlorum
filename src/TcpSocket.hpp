#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

extern "C" {
#include <netinet/in.h>
#include <sys/socket.h>
}
#include <cstdint>

#include "Protocol.hpp"
#include "Socket.hpp"

/////////////////////////// Declarations ///////////////////////////
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

template<typename Packet_t>
    requires PacketFormat<Packet_t>
class TcpListener;

template<typename Packet_t>
    requires PacketFormat<Packet_t>
class TcpConnection : public TcpSocket {
public:
    enum class Desire : char { qClose = 1, qRead = 1 << 1, qWrite = 1 << 2 };
    TcpConnection() = default;
    void connect(const SocketAddrIn& addr);
    void write_all();
    void read_all();
    inline Flag<Desire> desire() const noexcept {
        return desire_;
    }
    Flag<Desire> desire_;
    std::optional<Packet_t> try_request() noexcept;

protected:
    friend Packet_t;
    friend TcpListener<Packet_t>;
    TcpConnection(int fd) noexcept;

private:
    // [] Sending through a socket requires a storage to be linear
    // it's just memcpy'ing it
    // [] I was considering using a static storage as std::array<char>
    // but have chosen an std::vector<char> instead, because
    // in future a timer can be applied so when it expires
    // shrink_to_fit() can be executed
    // [] These bufs work as follows : data is written at `head`(`old_head`
    // preserved), when `head` hits trigger `head_trigger_dist` erasure in [0,
    // `old_head`] is performed, head -= old_head
    std::vector<char> recv_buf_;
    std::vector<char> send_buf_;
    std::uint32_t recv_buf_head_ = 0;
    std::uint32_t send_buf_head_ = 0;
    std::uint32_t lazy_erasure(std::vector<char>& buf,
                               std::uint32_t head) noexcept;
    static constexpr std::uint32_t head_trigger_dist
        = 64 * Packet_t::qPacketLen;
    static constexpr std::uint32_t buf_max_size_
        = head_trigger_dist + Packet_t::qPacketLen;
};

template<typename Packet_t>
    requires PacketFormat<Packet_t>
class TcpListener : public TcpSocket {
public:
    TcpListener() = default;
    void listen(int max_conn = qDefaultMaxConn);
    inline void start(const SocketAddrIn& addr, int max_conn = qDefaultMaxConn);
    TcpConnection<Packet_t> accept(SocketAddrIn* addr_ptr = nullptr);
    inline std::unique_ptr<TcpConnection<Packet_t>> accept_on_heap(
        SocketAddrIn* addr_ptr = nullptr);
};
}  // namespace dash

/////////////////////////// Implementation ///////////////////////////
////////////////////////// TcpConnection ////////////////////////////
namespace dash {

template<typename Packet_t>
    requires PacketFormat<Packet_t>
TcpConnection<Packet_t>::TcpConnection(int fd) noexcept : TcpSocket(fd) {
    set_nb();
    status_flags_ |= Status::qOpen;
}
template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpConnection<Packet_t>::connect(const SocketAddrIn& addr) {
    auto [c_addr, size] = addr.data();
    if (::connect(fd_, &c_addr, size) == -1) {
        throw dash::SocketException("Can't establish connection\n");
    } else {
        status_flags_ |= Status::qConnected;
    }
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpConnection<Packet_t>::write_all() {
    enum class ExcCase { qNone, qNotReady, qCantWrite, qEOF };
    ExcCase exc_case = ExcCase::qNone;
    std::uint32_t sz = send_buf_.size() - send_buf_head_;
    char* rbuf = send_buf_.data() + send_buf_head_;
    errno = 0;
    while (sz > 0) {
        ssize_t rv = ::write(*this, rbuf, sz);
        if (rv < 0 && errno == EAGAIN) {
            exc_case = ExcCase::qNotReady;
            break;
        } else if (rv < 0) {
            exc_case = ExcCase::qCantWrite;
            break;
        } else if (rv == 0) {
            exc_case = ExcCase::qEOF;
            break;
        }
        sz -= rv;
        rbuf += rv;
        send_buf_head_ += rv;
#ifdef DASH_DEBUG
        std::cout << std::format("WRITE : {} of {}\n", rv, sz);
#endif  // DASH_DEBUG
    }
    send_buf_head_ -= lazy_erasure(send_buf_, send_buf_head_);
    if (send_buf_head_ == 0) {
        desire_ *= Desire::qWrite;
        desire_ |= Desire::qRead;
    }
    switch (exc_case) {
    case ExcCase::qNone:
        break;
    case ExcCase::qNotReady:
        break;
    case ExcCase::qCantWrite: {
        desire_ *= Desire::qClose;
        throw dash::SocketException("Can't write to socket\n");
    } break;
    case ExcCase::qEOF: {
        desire_ *= Desire::qClose;
        if (recv_buf_.size() != 0) {
            throw dash::ConnectionEOF(
                "Can't write to socket: EOF/Closed, some data wan't sent\n");
        }  // else client closed correctly
    } break;
    }
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpConnection<Packet_t>::read_all() {
    enum class ExcCase { qNone, qNotReady, qCantRead, qEOF };
    ExcCase exc_case = ExcCase::qNone;
    errno = 0;
    std::array<char, buf_max_size_ / 4> wbuf_arr;
    std::uint32_t sz = wbuf_arr.size();
    char* wbuf = wbuf_arr.data();
    while (sz > 0) {
        ssize_t rv = ::read(*this, wbuf, sz);
        if (rv < 0 && errno == EAGAIN) {
            exc_case = ExcCase::qNotReady;
            break;
        } else if (rv < 0) {
            exc_case = ExcCase::qCantRead;
            break;
        } else if (rv == 0) {
            exc_case = ExcCase::qEOF;
            break;
        }
#ifdef DASH_DEBUG
        std::cout << std::format("READ : {} of capable {}\n", rv, sz);
#endif  // DASH_DEBUG
        sz -= rv;
        wbuf += rv;
        recv_buf_head_ += rv;
    }
    std::ranges::copy_n(
        wbuf_arr.cbegin(), wbuf_arr.size() - sz, std::back_inserter(recv_buf_));
#ifdef DASH_DEBUG
    auto recv_buf_old_size = recv_buf_.size();
#endif  // DASH_DEBUG
    recv_buf_head_ -= lazy_erasure(recv_buf_, recv_buf_head_);
#ifdef DASH_DEBUG
    std::cout << std::format("Before erasure : {}\nAfter erasure : {}\n",
                             recv_buf_old_size,
                             recv_buf_.size())
              << std::endl;
#endif  // DASH_DEBUG
    if (recv_buf_head_ == 0) {
        desire_ |= Desire::qWrite;
        desire_ *= Desire::qRead;
    }
    switch (exc_case) {
    case ExcCase::qNone:
        break;
    case ExcCase::qNotReady:
        break;
    case ExcCase::qCantRead: {
        desire_ *= Desire::qClose;
        throw dash::SocketException("Can't read from socket\n");
    } break;
    case ExcCase::qEOF: {
        desire_ *= Desire::qClose;
        if (recv_buf_.size() != 0) {
            throw dash::ConnectionEOF("Can't read from socket: EOF/Closed\n");
        }  // else client closed correctly
    } break;
    }
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
std::uint32_t TcpConnection<Packet_t>::lazy_erasure(
    std::vector<char>& buf, std::uint32_t head) noexcept {
    if (head == buf.size() && head >= Packet_t::qPacketLen) {
        buf.clear();
        return head;
    } else if (head > head_trigger_dist) {
        buf.erase(buf.begin(), buf.begin() + head);
        return head;
    }
    return 0;
}

// I chose to use 'std::optional + noexcept' instead of
// RVO + exceptions, considering there will be frequent
// packet assembly errors. Also, i think std::move for
// sizeof(dash::proto::Packet)~24 bytes is efficient enough
template<typename Packet_t>
    requires PacketFormat<Packet_t>
std::optional<Packet_t> TcpConnection<Packet_t>::try_request() noexcept {
    if (recv_buf_.size() < Packet_t::qHeaderLen) {
        // Packet is incomplete
        return {};
    }
    std::uint32_t msg_sz;
    std::memcpy(
        &msg_sz, recv_buf_.data() + recv_buf_head_, Packet_t::qHeaderLen);
    msg_sz = ::ntohl(msg_sz);
    if (recv_buf_.size() < recv_buf_head_ + Packet_t::qHeaderLen + msg_sz) {
        // Packet is incomplete
        return {};
    }
    auto recv_buf_head_old = recv_buf_head_;
    recv_buf_head_ += Packet_t::qHeaderLen + msg_sz;
    // extra copy performed??
    return std::optional<Packet_t>{std::in_place,
                                   recv_buf_.data() + recv_buf_head_old,
                                   Packet_t::qHeaderLen + msg_sz};
}

////////////////////////// TcpListener ////////////////////////////

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpListener<Packet_t>::start(const SocketAddrIn& addr, int max_conn) {
    bind(addr);
    listen(max_conn);
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpListener<Packet_t>::listen(int max_conn) {
    if (::listen(fd_, max_conn) == -1) {
        throw dash::SocketException("Can't start listen\n");
    } else {
        status_flags_ |= Status::qListening;
    }
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
TcpConnection<Packet_t> TcpListener<Packet_t>::accept(SocketAddrIn* addr_ptr) {
    if (!(status_flags_ & Status::qStarted)) {
        throw dash::IncorrectSocketStatus("Can't accept, not started");
    }
    int new_fd{0};
    if (addr_ptr == nullptr) {
        new_fd = ::accept(fd_, nullptr, nullptr);
    } else {
        new_fd = ::accept(fd_, &addr_ptr->addr_, &addr_ptr->size_);
    }
    if (new_fd == -1) {
        throw dash::SocketCreationError("Can't accept a new connection\n");
    }
    return {new_fd};
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
std::unique_ptr<TcpConnection<Packet_t>> TcpListener<Packet_t>::accept_on_heap(
    SocketAddrIn* addr_ptr) {
    // It can be done with std::make_unique but requires to friend it
    return std::unique_ptr<TcpConnection<Packet_t>>(
        new TcpConnection(accept(addr_ptr)));
}
}  // namespace dash

#endif  // TCP_SOCKET_HPP
