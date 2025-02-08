#include <stdexcept>

#include "../include/Server.hpp"
#include "../include/utils.hpp"

Server::Server(const char *port, const char *password)
    : _port(toInt(port)), _password(password) {
    if (errno != 0) {
        throw std::invalid_argument("Invalid port");
    }

    if (_port < 1024 || _port > 65535) {
        throw std::range_error("Port is out of range");
    }
}

Server::Server(const Server &rhs) : _port(rhs._port), _password(rhs._password) {
}

Server &Server::operator=(const Server &rhs) {
    if (this != &rhs) {
        _port = rhs._port;
        _password = rhs._password;
    }

    return *this;
}

Server::Server(Server &&rhs) noexcept
    : _port(rhs._port), _password(std::move(rhs._password)) {
}

Server &Server::operator=(Server &&rhs) noexcept {
    if (this != &rhs) {
        _port = rhs._port;
        _password = std::move(rhs._password);
    }

    return *this;
}

Server::~Server() {
}

void Server::initServer() {

}
