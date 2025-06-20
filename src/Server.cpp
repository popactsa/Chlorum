#include "Server.hpp"

#include <poll.h>
#include <sys/poll.h>
#include <sys/types.h>

#include <algorithm>
#include <ranges>

#include "Socket.hpp"

void Server::start(const dash::SocketAddrIn& addr, int max_conn) {
    lsock_.start(addr, max_conn);
    try {
        event_loop();
    } catch (const dash::SocketException& exc) {
        // It's actually a poll error, idk what to do with it
        std::cerr << exc.what() << std::endl;
        std::terminate();
    }
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
    } catch (...) {
        // Possible cases:
        // read_all : 1. can't read from a socket;
        //            2. EOF
        // All treatment performed in place
    }
    bool request_succeed = true;
    while (request_succeed) {
        auto req = conn->read_packet();
        if (req) {
#ifdef DASH_DEBUG
            std::cout << "CLIENT RECV : ";
            std::ranges::for_each(req->rmsg_range(),
                                  [](char c) { std::cout << c; });
            std::cout << std::endl;
            // Creating a response
            conn->write_packet(*req);
            conn->desire_ |= conn_t::Desire::qWrite;
            conn->desire_ *= conn_t::Desire::qRead;
            // Add app logic, transfering/sharing packet
#endif  // DASH_DEBUG
        } else {
            request_succeed = false;
        }
    }
    return conn;
}
std::unique_ptr<Server::conn_t> Server::handle_write(
    std::unique_ptr<conn_t>&& conn) {
    try {
        conn->write_all();
        conn->desire_ *= conn_t::Desire::qWrite;
        conn->desire_ |= conn_t::Desire::qRead;
    } catch (...) {
        // Possible cases:
        // write_all : 1. can't read from a socket;
        //             2. EOF
        // All treatment performed in place
    }
    return conn;
}

void Server::event_loop() {
    std::vector<std::unique_ptr<conn_t>> connections;
    std::vector<::pollfd> poll_args;
#ifdef DASH_DEBUG
    std::uint32_t connections_size_old = 0;
#endif  // DASH_DEBUG
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
            poll_args.data(), reinterpret_cast<nfds_t>(poll_args.size()), 5000);
        if (rv == 0) {
            throw dash::SocketException("Dead listening socket timeout(?)");
        }
        if (rv < 0 && errno != EINTR) {
            throw dash::SocketException("Poll error");
        }
        if (poll_args[0].revents) {
            try {
                connections.emplace_back(handle_accept());
            } catch (...) {
                // There are two possible cases: `lsock_` is not started or
                // new connection can't be accepted because of connections limit
                // or anything else. No treatment required
            }
        }
        for (std::uint32_t i{1}, current_conn{0}; i < poll_args.size(); ++i) {
            std::uint32_t ready = poll_args[i].revents;
            if (ready == 0) {
                continue;
            }
            if (ready & POLLIN) {
                assert(connections[current_conn]->desire()
                       & conn_t::Desire::qRead);
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
#ifdef DASH_DEBUG
        if (connections.size() != connections_size_old) {
            std::cout << std::format("Currently connections : {}",
                                     connections.size())
                      << std::endl;
            connections_size_old = connections.size();
        }
#endif  // DASH_DEBUG
    }
}
