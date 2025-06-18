#include "Protocol.hpp"

#include "error_handling.hpp"

namespace dash {
namespace proto {

Packet::Packet(std::string_view msg) {
    std::size_t msg_len = msg.size();
    if (msg_len > qMaxMsgLen) {
        throw dash::SocketException("Packet ctor message length exceeded\n");
    }
    std::memcpy(data_.data(), &msg_len, qHeaderLen);
    if (msg.back() != '\0') {
        std::memcpy(data_.data() + qHeaderLen, msg.data(), msg_len);
        *(data_.data() + qHeaderLen + msg_len + 1) = '\0';
    } else {
        std::strcpy(data_.data() + qHeaderLen, msg.data());
    }
}

Packet::Packet() noexcept {}

void receive_packet(TcpConnection& csock, Packet& wbuf) {
    csock.read_all(wbuf.wbuf(), Packet::qHeaderLen);
    std::uint32_t msg_len;
    std::memcpy(&msg_len, wbuf.wbuf(), Packet::qHeaderLen);
    if (msg_len > Packet::qMaxMsgLen) {
        throw dash::SocketException("Receiving message length exceeded\n");
        // What to do with the remaining message?
    }
    try {
        csock.read_all(wbuf.wbuf() + Packet::qHeaderLen, msg_len);
    } catch (const dash::ConnectionEOF& exc) {
        throw;
    }
#ifdef DASH_DEBUG
    std::cout << std::format(
        "Server received : {}\n",
        std::string_view(wbuf.wbuf() + Packet::qHeaderLen,
                         wbuf.wbuf() + Packet::qHeaderLen + msg_len));
#endif  // DASH_DEBUG
}

void send_packet(TcpConnection& csock, const Packet& rbuf) {
    std::uint32_t msg_len;
    std::memcpy(&msg_len, rbuf.rbuf(), Packet::qHeaderLen);
    if (msg_len > Packet::qMaxMsgLen) {
        throw dash::SocketException("Sending message length exceeded\n");
        // What to do with the remaining message?
    }
    csock.write_all(rbuf.rbuf(), Packet::qHeaderLen);
    csock.write_all(rbuf.rbuf() + Packet::qHeaderLen, msg_len);
}

}  // namespace proto
}  // namespace dash
