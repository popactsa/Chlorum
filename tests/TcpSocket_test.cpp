#include <arpa/inet.h>
#include <gtest/gtest.h>

#include <condition_variable>
#include <mutex>
#include <thread>

#include "Protocol.hpp"
#include "TcpConnection.hpp"
#include "TcpListener.hpp"

extern sockaddr_in local_addr;

TEST(TcpSocketTest, TcpListener_BaseScenario) {
    using namespace dash;
    using Packet_t = dash::proto::Packet;
    using enum Socket::Status;

    TcpListener<Packet_t> lsock;
    lsock.start(local_addr);
    EXPECT_TRUE(lsock.status_flags()
                == (Flag<Socket::Status>() | qNonBlocking | qReusable
                    | qListening | qBound));
}

TEST(TcpSocketTest, TcpListener_CantAcceptWithNoBind) {
    using namespace dash;
    using Packet_t = dash::proto::Packet;
    using enum Socket::Status;

    TcpListener<Packet_t> lsock;
    EXPECT_TRUE(lsock.status_flags() == (Flag<Socket::Status>() | qOpen));
    EXPECT_NO_THROW(lsock.listen());
    EXPECT_EQ(lsock.accept_on_heap(), nullptr);
}

TEST(TcpSocketTest, TcpConnection_ThrowWhenNoListener) {
    using namespace dash;
    using Packet_t = dash::proto::Packet;
    using enum Socket::Status;
    using namespace std::chrono_literals;

    TcpConnection<Packet_t> csock;
    EXPECT_TRUE(csock.status_flags() == (Flag<Socket::Status>() | qOpen));
    ASSERT_THROW(csock.connect(local_addr), dash::SocketException);
}

// namespace {
// std::mutex              mtx;
// std::condition_variable cv;
// bool                    server_ready = false;
//
// template<typename Packet_t>
// void StartLSockAndWaitAccept(const dash::SocketAddrIn& addr) {
//     std::lock_guard<std::mutex> lock(mtx);
//
//     dash::TcpListener<Packet_t> lsock;
//     lsock.start(addr);
//     server_ready = true;
//     cv.notify_one();
//     while (lsock.accept_on_heap() == nullptr) {}
// }
//
// template<typename Packet_t>
// void ConnectCSock(const dash::SocketAddrIn& addr) {
//     using namespace dash;
//     using enum Socket::Status;
//     dash::TcpConnection<Packet_t> csock;
//     std::unique_lock<std::mutex>  lock(mtx);
//     cv.wait(lock, []() { return server_ready; });
//     while (true) {
//         try {
//             csock.connect(addr);
//             break;
//         } catch (...) {
//             using namespace std::chrono_literals;
//             std::this_thread::sleep_for(50ms);
//         }
//     }
//     EXPECT_TRUE(csock.status_flags() == (Flag<Socket::Status>() |
//     qConnected));
// }
// };  // namespace
//
// TEST(TcpSocketTest, TcpConnection_BaseScenario) {
//     using namespace dash;
//     using Packet_t = dash::proto::Packet;
//
//     std::vector<std::jthread> threads;
//     threads.emplace_back(StartLSockAndWaitAccept<Packet_t>, local_addr);
//     threads.emplace_back(ConnectCSock<Packet_t>, local_addr);
// }
