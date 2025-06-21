#ifndef TCP_SOCKET_HPP
#define TCP_SOCKET_HPP

#include <cerrno>
#include <utility>

#include "auxiliary_functions.hpp"
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
    static constexpr std::uint32_t qHeadTriggerDist = 30;
    // = qBufMaxSize - Packet_t::qPacketLen;
    static constexpr std::uint32_t qBufInitSize = qBufMaxSize / 8;
    static constexpr std::uint32_t qMaxFailedReadAttempts = 10;
    static_assert(qBufMaxSize >= qBufInitSize);
    static_assert(qHeadTriggerDist <= qBufMaxSize - Packet_t::qPacketLen);

    TcpConnection();
    void connect(const SocketAddrIn& addr);
    void write_all();
    void read_all();
    inline Flag<Desire> desire() const noexcept {
        return desire_;
    }
    Flag<Desire> desire_;
    void write_packet(const Packet_t& packet);
    std::optional<Packet_t> read_packet() noexcept;
    constexpr std::uint32_t recv_diff() const noexcept {
        return recv_buf_tail_ - recv_buf_head_;
    }
    constexpr std::uint32_t send_diff() const noexcept {
        return send_buf_tail_ - send_buf_head_;
    }

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
    // [] recv_buf_ and send_buf_ work as queues:
    // {.....<head------------tail>........}
    // Data is taken from head, added from tail
    std::vector<char> recv_buf_;
    std::vector<char> send_buf_;
    std::uint32_t recv_buf_head_ = 0;
    std::uint32_t send_buf_head_ = 0;
    std::uint32_t recv_buf_tail_ = 0;
    std::uint32_t send_buf_tail_ = 0;
    std::uint32_t failed_attempts_to_read = 0;
    void lazy_erasure(std::vector<char>& buf) noexcept;
    void expand_buf(std::vector<char>& buf);
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
void TcpConnection<Packet_t>::expand_buf(std::vector<char>& buf) {
    buf.resize(buf.size() * 2);
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpConnection<Packet_t>::write_packet(const Packet_t& packet) {
    if (send_buf_tail_ + packet.size() > send_buf_.size()) {
        expand_buf(send_buf_);
    }
    std::memcpy(
        send_buf_.data() + send_buf_tail_, packet.rdata(), packet.size());
    send_buf_tail_ += packet.size();
}

// I chose to use 'std::optional + noexcept' instead of
// RVO + exceptions, considering there will be frequent
// packet assembly errors. Also, i think std::move for
// sizeof(dash::proto::Packet)~24 bytes is efficient enough
template<typename Packet_t>
    requires PacketFormat<Packet_t>
std::optional<Packet_t> TcpConnection<Packet_t>::read_packet() noexcept {
    if (failed_attempts_to_read == qMaxFailedReadAttempts) {
        recv_buf_.clear();
        recv_buf_head_ = 0;
        recv_buf_tail_ = 0;
        failed_attempts_to_read = 0;
        return {};
    }
    try {
        std::optional<Packet_t> result(
            std::in_place, recv_buf_, recv_buf_head_, recv_buf_tail_);
        failed_attempts_to_read = 0;
        lazy_erasure(recv_buf_);
        return result;
    } catch (const dash::proto::PacketException& exc) {
        failed_attempts_to_read++;
        return {};
    }
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpConnection<Packet_t>::write_all() {
    enum class ExcCase { qNone, qNotReady, qCantWrite, qEOF };
    ExcCase exc_case = ExcCase::qNone;
    std::uint32_t sz = send_diff();
    char* buf = send_buf_.data() + send_buf_head_;
    errno = 0;
    if (sz == 0) {
        desire_ *= Desire::qWrite;
        desire_ |= Desire::qRead;
        return;
    }
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
#ifdef DASH_DEBUG
        dash::rc_free_print(
            std::string(std::format("WRITE : {} of {}\n", rv, sz)));
#endif  // DASH_DEBUG
        sz -= rv;
        buf += rv;
        send_buf_head_ += rv;
    }
    lazy_erasure(send_buf_);
    switch (exc_case) {
    case ExcCase::qNone:
        break;
    case ExcCase::qNotReady:
        break;
    case ExcCase::qCantWrite: {
        desire_ |= Desire::qClose;
    } break;
    case ExcCase::qEOF: {
        desire_ |= Desire::qClose;
    } break;
    }
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpConnection<Packet_t>::read_all() {
    enum class ExcCase { qNone, qNotReady, qCantRead, qEOF };
    ExcCase exc_case = ExcCase::qNone;
    errno = 0;
    if (recv_buf_.size() - recv_buf_tail_ < Packet_t::qPacketLen) {
        expand_buf(recv_buf_);
    }
    std::uint32_t sz = recv_buf_.size() - recv_buf_tail_;
    char* buf = recv_buf_.data() + recv_buf_tail_;
    for (ssize_t rv = 1; sz > 0 && rv != 0;) {
        rv = ::read(*this, buf, sz);
        if (rv < 0 && errno == EAGAIN) {
            exc_case = ExcCase::qNotReady;
            break;
        } else if (rv < 0) {
            exc_case = ExcCase::qCantRead;
            break;
        } else if (rv == 0) {
            // unexpected eof if it is a first attempt to read
            exc_case = ExcCase::qEOF;
            break;
        }
#ifdef DASH_DEBUG
        dash::rc_free_print(
            std::string(std::format("READ : {} of capable {}\n", rv, sz)));
#endif  // DASH_DEBUG
        sz -= rv;
        buf += rv;
        recv_buf_tail_ += rv;
    }
    switch (exc_case) {
    case ExcCase::qNone:
        break;
    case ExcCase::qNotReady:
        break;
    case ExcCase::qCantRead: {
        desire_ |= Desire::qClose;
    } break;
    case ExcCase::qEOF: {
        desire_ |= Desire::qClose;
    } break;
    }
}

template<typename Packet_t>
    requires PacketFormat<Packet_t>
void TcpConnection<Packet_t>::lazy_erasure(std::vector<char>& buf) noexcept {
    // Pretty not extensible
    std::uint32_t& head = &buf == &recv_buf_ ? recv_buf_head_ : send_buf_head_;
    std::uint32_t& tail = &buf == &recv_buf_ ? recv_buf_tail_ : send_buf_tail_;
    assert(tail >= head);
    if (tail == head) {
        head = 0;
        tail = 0;
    } else if (head > qHeadTriggerDist) {
        buf.erase(buf.begin(), buf.begin() + head);
        tail -= head;
        head = 0;
    }
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
