#include <iostream>
#include <stdexcept>

#include "../include/Server.hpp"
#include "../include/utils.hpp"

Server::Server(const char *port, const char *password)
    : _port(toInt(port)), _password(password) {
        if (errno != 0) {
            throw std::invalid_argument("Invalid port");
        }

        if (_port < 1024 || _port > 65535) {
           throw std::range_error("Port is out of range") ;
        }
}

Server::Server(const Server & /*rhs*/) {
    std::cout << "Default copy constructor for Server is called" << '\n';
}

Server &Server::operator=(const Server &rhs) {
    std::cout << "Copy assigment constructor for Server is called" << '\n';
    if (this != &rhs) {
    }

    return *this;
}

Server::Server(Server && /*rhs*/) noexcept {
    std::cout << "Default move constructor for Server is called" << '\n';
}

Server &Server::operator=(Server &&rhs) noexcept {
    std::cout << "Move assigment constructor for Server is called" << '\n';
    if (this != &rhs) {
    }

    return *this;
}

Server::~Server() {
    std::cout << "Default destructor for Server is called" << '\n';
}
