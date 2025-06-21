#include <arpa/inet.h>

#include <thread>

#include "Client.hpp"
#include "Protocol.hpp"
#include "Server.hpp"
#include "Socket.hpp"

static sockaddr_in addr;

void host_server() {
    Server server;
    server.start(addr);
}

void run_client_auto([[maybe_unused]] int arg) {
    Client<dash::proto::Packet> client(addr);
    client.do_something(arg);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8082);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    std::jthread server_thread(host_server);
    std::vector<std::jthread> threads;
    for (int i{0}; i < 4; ++i) {
        threads.emplace_back(run_client_auto, i);
    }
    threads.clear();
    using namespace std::chrono_literals;
    std::cout << "Sleeping.....5 sec" << std::endl;
    std::this_thread::sleep_for(5000ms);
    for (int i{0}; i < 4; ++i) {
        threads.emplace_back(run_client_auto, i);
    }
    for (int i{0}; i < 4; ++i) {
        threads[i].join();
    }
    std::cout << "Done!" << std::endl;
    return 0;
}
