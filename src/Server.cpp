#include <iostream>

#include <unistd.h>

#include "../include/Server.hpp"
#include "../include/utils.hpp"

Server::Server(const char *port, const char *password)
    : _port(toInt(port)), _password(password), _socket(-1), _sockaddr({}) {
    if (errno != 0) {
        throw std::invalid_argument("Invalid port");
    }

    if (_port < LOWEST_PORT || _port > HIGHEST_PORT) {
        throw std::range_error("Port is out of range");
    }

    if (_password.length() == 0) {
        throw std::invalid_argument("password can not be empty");
    }

    startServer();
}

Server::Server(const Server &rhs)
    : _port(rhs._port), _password(rhs._password), _socket(rhs._socket),
      _sockaddr(rhs._sockaddr) {
}

Server &Server::operator=(const Server &rhs) {
    if (this != &rhs) {
        _port = rhs._port;
        _password = rhs._password;
        _socket = rhs._socket;
        _sockaddr = rhs._sockaddr;
    }

    return *this;
}

Server::Server(Server &&rhs) noexcept
    : _port(rhs._port), _password(std::move(rhs._password)),
      _socket(rhs._socket), _sockaddr(rhs._sockaddr) {
}

Server &Server::operator=(Server &&rhs) noexcept {
    if (this != &rhs) {
        _port = rhs._port;
        _password = std::move(rhs._password);
        _socket = rhs._socket;
        _sockaddr = rhs._sockaddr;
    }

    return *this;
}

Server::~Server() {
    close(_socket);
}

bool Server::startServer() {
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > _socket) {
        std::cerr << "Failed to create a socket" << '\n';
        return false;
    }

    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_port = htons((u_int16_t)_port);
    _sockaddr.sin_addr.s_addr = INADDR_ANY;
    _addrlen = sizeof(_sockaddr);

    if (0 > bind(_socket, (struct sockaddr *)&_sockaddr, _addrlen)) {
        std::cerr << "Failed to bind" << '\n';
        return false;
    }

    if (0 > listen(_socket, SOMAXCONN)) {
        std::cerr << "Failed to listen" << '\n';
        return false;
    }

    return true;
}
