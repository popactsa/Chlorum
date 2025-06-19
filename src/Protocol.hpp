#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <ranges>
#include <string_view>

namespace dash {
namespace proto {

template<typename T>
concept LinearConstContainer = requires(const T t) {
    requires std::contiguous_iterator<decltype(std::cbegin(t))>;
    { t.data() } -> std::same_as<const typename T::value_type*>;
    { t.size() } -> std::convertible_to<std::uint32_t>;
};
struct Packet {
    static constexpr std::uint32_t qHeaderLen = sizeof(std::uint16_t);
    static constexpr std::uint32_t qMaxMsgLen
        = std::numeric_limits<std::uint16_t>::max();
    static constexpr std::uint32_t qPacketLen = qHeaderLen + qMaxMsgLen;
    Packet() noexcept;
    template<typename T>
        requires LinearConstContainer<T>
    Packet(const T& msg_owner);
    template<typename T>
    Packet(const T* msg, std::uint32_t sz);
    Packet(std::string_view msg);
    constexpr const char* rbuf() const noexcept;
    constexpr char* wbuf() noexcept;
    constexpr const char* rmsg() const noexcept;
    constexpr char* wmsg() noexcept;
    constexpr std::uint32_t msg_sz() const noexcept;
    constexpr auto rmsg_range() const noexcept;
    std::array<char, qPacketLen> data_;
    std::uint32_t msg_sz_;
};

///////////////////////// Exceptions /////////////////////////
class PacketException : public std::exception {
protected:
    std::string msg_;

public:
    explicit PacketException(const std::string& msg) : msg_(msg) {}
    const char* what() const noexcept override {
        return msg_.c_str();
    }
};
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

constexpr const char* Packet::rmsg() const noexcept {
    return data_.data() + qHeaderLen;
}

constexpr char* Packet::wmsg() noexcept {
    return data_.data() + qHeaderLen;
}

constexpr std::uint32_t Packet::msg_sz() const noexcept {
    return msg_sz_;
}

template<typename T>
Packet::Packet(const T* msg, std::uint32_t msg_sz) {
    if (msg_sz > qMaxMsgLen) {
        throw PacketException("Packet ctor message length exceeded\n");
    }
    msg_sz_ = msg_sz;
    std::memcpy(data_.data(), &msg_sz, qHeaderLen);
    std::memcpy(data_.data() + qHeaderLen, msg, msg_sz);
}

template<typename T>
    requires LinearConstContainer<T>
Packet::Packet(const T& msg_owner) :
    Packet(msg_owner.data(), msg_owner.size()) {}

constexpr auto Packet::rmsg_range() const noexcept {
    return std::ranges::subrange(data_.begin() + qHeaderLen,
                                 data_.begin() + qHeaderLen + msg_sz_);
}

}  // namespace proto
}  // namespace dash

#endif  // PROTOCOL_HPP
