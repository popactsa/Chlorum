#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "Server.hpp"

class Database : public Server {
public:
    using Server::Server;
    using Server::start;
};

#endif  // DATABASE_HPP
