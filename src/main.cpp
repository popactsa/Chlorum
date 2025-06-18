#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <thread>

#include "Server.hpp"
extern "C" {
#include <arpa/inet.h>
}

#include "Client.hpp"
#include "Protocol.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "TcpSocket.hpp"

static sockaddr_in addr;

void host_server() {
    Server server(addr);
    server.start();
}

void run_client() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    Client client(addr);
    dash::proto::Packet packet{"that's what she said"};
    client.send_packet(packet);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    std::jthread server_thread(host_server);
    std::jthread client_thread(run_client);
    return 0;
}
