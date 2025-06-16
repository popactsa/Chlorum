#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
extern "C" {
#include "arpa/inet.h"
}

#include "Socket.hpp"
#include "TcpSocket.hpp"
#include "error_handling.hpp"

void host_server() {
    TcpListener server;
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;
    server.start(addr);
    std::cout << "Server started" << std::endl;
    auto client = server.accept_client();
    std::array<char, 1024> buf;
    std::size_t recv_buf_size = client.recv(buf.data(), buf.size());
    std::cout << "Received : " << recv_buf_size << " bytes" << std::endl;
    std::ranges::for_each(buf, [](char c) { std::cout << c; });
    std::cout << std::endl;
}

void run_client() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    TcpConnection conn;
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    conn.connect(addr);
    conn.send("dasha is the coolest girl ever", 30);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    std::jthread server_thread(host_server);
    std::jthread client_thread(run_client);
    return 0;
}
