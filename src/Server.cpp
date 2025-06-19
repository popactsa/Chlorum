#include "Server.hpp"

#include <poll.h>
#include <sys/poll.h>
#include <sys/types.h>

#include <algorithm>
#include <ranges>

#include "Socket.hpp"

Server::Server(const dash::SocketAddrIn& addr) {
    lsock_.bind(addr);
}

void Server::start(const dash::SocketAddrIn& addr, int max_conn) {
    if (!(lsock_.status_flags() & dash::Socket::Status::qBound)) {
        lsock_.start(addr, max_conn);
    } else {
        lsock_.listen(max_conn);
    }
    event_loop();
}

void Server::start(int max_conn) {
    if (!(lsock_.status_flags() & dash::Socket::Status::qBound)) {
        throw dash::SocketException("Trying to start without a binding\n");
    } else {
        lsock_.listen(max_conn);
    }
    event_loop();
}

std::unique_ptr<Server::conn_t> Server::handle_accept() {
    std::unique_ptr<conn_t> client = lsock_.accept_on_heap();
    client->set_nb();
    client->desire_ = conn_t::Desire::qRead;
    return client;
}

std::unique_ptr<Server::conn_t> Server::handle_read(
    std::unique_ptr<conn_t>&& conn) {
    try {
        conn->read_all();
        bool request_succeed = true;
        while (request_succeed) {
            auto req = conn->try_request();
            if (req) {
                std::ranges::for_each(req->rmsg_range(),
                                      [](char c) { std::cout << c; });
                std::cout << std::endl;
            } else {
                request_succeed = false;
            }
        }
    } catch (...) {}
    // Implicitly choose move-ctor
    return conn;
}
std::unique_ptr<Server::conn_t> Server::handle_write(
    std::unique_ptr<conn_t>&& conn) {
    try {
        conn->write_all();
    } catch (...) {}
    // Implicitly choose move-ctor
    return conn;
}

void Server::event_loop() {
    std::vector<std::unique_ptr<conn_t>> connections;
    std::vector<::pollfd> poll_args;
    while (true) {
        poll_args.clear();
        poll_args.emplace_back(lsock_, POLLIN, 0);
        for (const auto& conn : connections) {
            poll_args.emplace_back(*conn, POLLERR, 0);
            if (conn->desire() & conn_t::Desire::qRead) {
                poll_args.back().events |= POLLIN;
            }
            if (conn->desire() & conn_t::Desire::qWrite) {
                poll_args.back().events |= POLLOUT;
            }
        }
        errno = 0;
        int rv = ::poll(
            poll_args.data(), reinterpret_cast<nfds_t>(poll_args.size()), -1);
        if (rv < 0 && errno != EINTR) {
            throw dash::SocketException("Poll error");
        }
        if (poll_args[0].revents) {
            connections.emplace_back(handle_accept());
            // handle exceptions
        }
        for (std::uint32_t i{1}, current_conn{0}; i < poll_args.size(); ++i) {
            std::uint32_t ready = poll_args[i].revents;
            if (ready == 0) {
                continue;
            }
            if (ready & POLLIN) {
                assert(connections[current_conn]->desire()
                       & conn_t::Desire::qRead);
                // maybe it's more clever to use shared_ptr and don't return
                // anything
                connections[current_conn]
                    = handle_read(std::move(connections[current_conn]));
            }
            if (ready & POLLOUT) {
                assert(connections[current_conn]->desire()
                       & conn_t::Desire::qWrite);
                connections[current_conn]
                    = handle_write(std::move(connections[current_conn]));
            }
            if (ready & POLLERR
                || connections[current_conn]->desire()
                       & conn_t::Desire::qClose) {
                connections[current_conn]->close();
                auto found = std::ranges::find_if(
                    connections | std::views::reverse,
                    [](const auto& ptr) { return ptr == nullptr; });
                if (found != connections.rend()) {
                    std::swap(connections[current_conn], *found);
                }
                connections.pop_back();
            } else {
                current_conn++;
            }
        }
    }
}
