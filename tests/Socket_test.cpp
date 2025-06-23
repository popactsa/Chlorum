#include "Socket.hpp"

#include <arpa/inet.h>
#include <gtest/gtest.h>

TEST(SocketTest, Socket_BaseScenario) {
    using namespace dash;
    using enum Socket::Status;
    Socket      sock(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in local_addr;
    local_addr.sin_family      = AF_INET;
    local_addr.sin_port        = htons(8080);
    local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    EXPECT_TRUE(sock.status_flags() == (Flag<Socket::Status>() | qOpen));
    sock.bind(local_addr);
    EXPECT_TRUE(sock.status_flags()
                == (Flag<Socket::Status>() | qOpen | qBound | qReusable));

    sock.set_nb();
    EXPECT_TRUE(sock.status_flags()
                == (Flag<Socket::Status>() | qOpen | qBound | qNonBlocking
                    | qReusable));
    sock.close();
    EXPECT_TRUE(
        sock.status_flags()
        == (Flag<Socket::Status>() | qBound | qNonBlocking | qReusable));
}
