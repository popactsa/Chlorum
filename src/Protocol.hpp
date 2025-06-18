#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <array>
#include <cstdint>
#include <limits>
#include <string_view>

#include "TcpSocket.hpp"

namespace dash {
namespace proto {
struct Packet {
    static constexpr std::uint32_t qHeaderLen = sizeof(int);
    static constexpr std::uint32_t qMaxMsgLen = 4096;
    static constexpr std::uint32_t qPacketLen = qHeaderLen + qMaxMsgLen;
    Packet() noexcept;
    Packet(std::string_view msg);
    constexpr const char* rbuf() const noexcept;
    constexpr char* wbuf() noexcept;
    std::array<char, qPacketLen> data_;
};
void receive_packet(TcpConnection& csock, Packet& wbuf);
void send_packet(TcpConnection& csock, const Packet& rbuf);
}  // namespace proto
}  // namespace dash

///////////////// Implementation /////////////////

namespace dash {
namespace proto {
constexpr const char* Packet::rbuf() const noexcept {
    return data_.data();
}

constexpr char* Packet::wbuf() noexcept {
    return data_.data();
}
}  // namespace proto
}  // namespace dash

#endif  // PROTOCOL_HPP
