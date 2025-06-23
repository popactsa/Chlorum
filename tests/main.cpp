#include <arpa/inet.h>
#include <gtest/gtest.h>

sockaddr_in local_addr;

int main(int argc, char **argv) {
    local_addr.sin_family      = AF_INET;
    local_addr.sin_port        = htons(8080);
    local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
