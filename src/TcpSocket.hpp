#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#include <cerrno>
#include <utility>
extern "C" {
#include <netinet/in.h>
#include <sys/socket.h>
}
#include <cstdint>
#include <iostream>

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
    static constexpr std::uint32_t qBufMaxSize = 64 * Packet_t::qPacketLen;
    static constexpr std::uint32_t qTailTriggerDist
        = qBufMaxSize - Packet_t::qPacketLen;
    static constexpr std::uint32_t qBufInitSize = qBufMaxSize / 8;
    static_assert(qBufMaxSize >= qBufInitSize);
    static_assert(qTailTriggerDist <= qBufMaxSize - Packet_t::qPacketLen);

    TcpConnection();
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
    TcpConnection(int fd);

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
    std::uint32_t recv_buf_tail_ = 0;
    std::uint32_t send_buf_tail_ = 0;
    void lazy_erasure(std::vector<char>& buf, std::uint32_t& head,
                      std::uint32_t& tail) noexcept;
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
TcpConnection<Packet_t>::TcpConnection() : TcpSocket() {
    recv_buf_.resize(qBufInitSize);
    send_buf_.resize(qBufInitSize);
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
TcpConnection<Packet_t>::TcpConnection(int fd) : TcpSocket(fd) {
    recv_buf_.resize(qBufInitSize);
    send_buf_.resize(qBufInitSize);
}
template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpConnection<Packet_t>::connect(const SocketAddrIn& addr) {
    auto [c_addr, size] = addr.data();
    errno = 0;
    if (::connect(fd_, &c_addr, size) == -1) {
        if (errno != EINPROGRESS) {
            throw dash::SocketException("Can't establish connection\n");
        }
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
    char* buf = send_buf_.data() + send_buf_head_;
    errno = 0;
    while (sz > 0) {
        ssize_t rv = ::write(*this, buf, sz);
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
        buf += rv;
        send_buf_head_ += rv;
#ifdef DASH_DEBUG
        std::cout << std::format("WRITE : {} of {}\n", rv, sz);
#endif  // DASH_DEBUG
    }
    lazy_erasure(send_buf_, send_buf_head_, send_buf_tail_);
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
    if (recv_buf_.size() - recv_buf_head_ < Packet_t::qPacketLen) {
        recv_buf_.resize(recv_buf_.size() * 2);
    }
    std::uint32_t sz = recv_buf_.size() - recv_buf_head_;
    char* buf = recv_buf_.data() + recv_buf_head_;
    bool first_try = true;
    for (ssize_t rv = 1; sz > 0 && rv != 0;) {
        rv = ::read(*this, buf, sz);
        if (rv < 0 && errno == EAGAIN) {
            exc_case = ExcCase::qNotReady;
            break;
        } else if (rv < 0) {
            exc_case = ExcCase::qCantRead;
            break;
        } else if (rv == 0 && first_try) {
            // unexpected eof if it is not a first attempt to read
            exc_case = ExcCase::qEOF;
            break;
        }
        first_try = false;
#ifdef DASH_DEBUG
        std::cout << std::format("READ : {} of capable {}\n", rv, sz);
#endif  // DASH_DEBUG
        sz -= rv;
        buf += rv;
        recv_buf_head_ += rv;
    }
    lazy_erasure(recv_buf_, recv_buf_head_, recv_buf_tail_);
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
void TcpConnection<Packet_t>::lazy_erasure(std::vector<char>& buf,
                                           std::uint32_t& head,
                                           std::uint32_t& tail) noexcept {
    if (head == buf.size() && head >= Packet_t::qPacketLen) {
        buf.clear();
        head = 0;
        tail = 0;
    } else if (tail > qTailTriggerDist) {
        buf.erase(buf.begin(), buf.begin() + tail);
        head -= tail;
        tail = 0;
    }
}

// I chose to use 'std::optional + noexcept' instead of
// RVO + exceptions, considering there will be frequent
// packet assembly errors. Also, i think std::move for
// sizeof(dash::proto::Packet)~24 bytes is efficient enough
template<typename Packet_t>
    requires PacketFormat<Packet_t>
std::optional<Packet_t> TcpConnection<Packet_t>::try_request() noexcept {
    if (recv_buf_head_ - recv_buf_head_ < Packet_t::qHeaderLen) {
        // Packet is incomplete
        return {};
    }
    std::uint32_t msg_sz;
    std::memcpy(
        &msg_sz, recv_buf_.data() + recv_buf_head_, Packet_t::qHeaderLen);
    msg_sz = ::ntohl(msg_sz);
    if (recv_buf_head_ - recv_buf_tail_ < Packet_t::qHeaderLen + msg_sz) {
        // Packet is incomplete
        return {};
    }
    std::optional<Packet_t> result(std::in_place,
                                   recv_buf_.data() + recv_buf_head_,
                                   Packet_t::qHeaderLen + msg_sz);
    recv_buf_tail_ += Packet_t::qHeaderLen + msg_sz;
    lazy_erasure(recv_buf_, recv_buf_head_, recv_buf_tail_);
    return result;
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
