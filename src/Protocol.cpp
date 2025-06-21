#include "Protocol.hpp"

namespace dash {
namespace proto {

Packet::Packet() noexcept {}

Packet::Packet(std::string_view msg) {
    std::uint32_t msg_len(msg.size());
    std::memcpy(data_.data(), &msg_len, Packet::qHeaderLen);
    std::memcpy(data_.data() + Packet::qHeaderLen, msg.data(), msg_len);
}

}  // namespace proto
}  // namespace dash
