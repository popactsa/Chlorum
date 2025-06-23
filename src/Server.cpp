#include "Server.hpp"

#include <poll.h>
#include <sys/poll.h>
#include <sys/types.h>

#include <algorithm>

#include "Socket.hpp"
#include "auxiliary_functions.hpp"

Server::Server() {
    lsock_.set_nb();
}

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
    if (std::unique_ptr<conn_t> client = lsock_.accept_on_heap();
        client != nullptr) {
        client->set_nb();
        client->desire_ = conn_t::Desire::qRead;
        return client;
    } else {
        return nullptr;
    }
}

std::unique_ptr<Server::conn_t> Server::handle_read(
    std::unique_ptr<conn_t>&& conn) {
    conn->read_all();
    for (bool request_succeed = true; request_succeed;) {
        auto req = conn->read_packet();
        if (req) {
            // Generating an echo
            conn->write_packet(*req);
#ifdef DASH_DEBUG
            std::string msg;
            for (char c : req->rmsg_range()) {
                msg += c;
            }
            msg = std::format("SERVER RECV: {}\n", msg);
            dash::rc_free_print(msg);
#endif  // DASH_DEBUG
        } else {
            request_succeed = false;
        }
    }
    if (conn->send_diff() != 0) {
        conn->desire_ |= conn_t::Desire::qWrite;
        conn->desire_ *= conn_t::Desire::qRead;
    }
    return conn;
}

std::unique_ptr<Server::conn_t> Server::handle_write(
    std::unique_ptr<conn_t>&& conn) {
    conn->write_all();
    if (conn->send_diff() == 0) {
        conn->desire_ *= conn_t::Desire::qWrite;
        conn->desire_ |= conn_t::Desire::qRead;
    }
    return conn;
}

void Server::event_loop() {
    std::vector<std::unique_ptr<conn_t>> connections;
    std::vector<::pollfd>                poll_args;
    auto close_connection = [&connections](std::uint32_t i) {
        connections[i]->close();
        std::swap(connections[i], connections.back());
        connections.pop_back();
    };
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
        errno  = 0;
        int rv = ::poll(
            poll_args.data(), reinterpret_cast<nfds_t>(poll_args.size()), -1);
        if (rv < 0 && errno != EINTR) {
            throw dash::SocketException("Poll error");
        }
        if (poll_args[0].revents) {
            connections.emplace_back(handle_accept());
            if (connections.back() == nullptr) {
                connections.pop_back();
            }
        }
        for (std::uint32_t i{1}, current_conn{0}; i < poll_args.size(); ++i) {
            std::uint32_t ready = poll_args[i].revents;
            if (ready == 0) {
                continue;
            }
            if (ready & POLLERR) {
                close_connection(current_conn);
                continue;
            }
            if (ready & POLLIN
                && connections[current_conn]->desire()
                       & conn_t::Desire::qRead) {
                connections[current_conn]
                    = handle_read(std::move(connections[current_conn]));
            }
            if (ready & POLLOUT
                && connections[current_conn]->desire()
                       & conn_t::Desire::qWrite) {
                connections[current_conn]
                    = handle_write(std::move(connections[current_conn]));
            }
            if (connections[current_conn]->desire() & conn_t::Desire::qClose) {
                close_connection(current_conn);
            } else {
                current_conn++;
            }
        }
#ifdef DASH_DEBUG
        if (connections.size() != connections_size_old) {
            dash::rc_free_print(std::string(
                std::format("======Currently connections : {}======\n",
                            connections.size())));
            connections_size_old = connections.size();
        }
#endif  // DASH_DEBUG
    }
}
