#include <arpa/inet.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <thread>

#include "Client.hpp"
#include "Protocol.hpp"
#include "Server.hpp"
#include "Socket.hpp"

static sockaddr_in addr;

void host_server() {
    Server server(addr);
    server.start();
}

void run_client_auto(int arg) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Client<dash::proto::Packet> client(addr);
    for (int i{0}; i < 10; ++i) {
        std::string msg = std::format("that's what she said : {} - {}", i, arg);
        dash::proto::Packet packet(msg);
        client.send_packet(packet);
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8085);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    std::jthread server_thread(host_server);
    std::vector<std::jthread> threads;
    for (int i{0}; i < 1; ++i) {
        threads.emplace_back(run_client_auto, i);
    }
    return 0;
}
