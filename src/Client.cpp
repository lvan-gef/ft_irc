#include <iostream>

#include "../include/Client.hpp"

Client::Client() {
    std::cout << "Default constructor for Client is called" << '\n';
}

Client::Client(const Client & /*rhs*/) {
    std::cout << "Default copy constructor for Client is called" << '\n';
}

Client &Client::operator=(const Client &rhs) {
    std::cout << "Copy assigment constructor for Client is called" << '\n';
    if (this != &rhs) {

    }

    return *this;
}

Client::Client(Client &&  /*rhs*/) noexcept {
    std::cout << "Default move constructor for Client is called" << '\n';
}

Client &Client::operator=(Client && rhs) noexcept {
    std::cout << "Move assigment constructor for Client is called" << '\n';
    if (this != &rhs) {

    }

    return *this;
}

Client::~Client() {
    std::cout << "Default destructor for Client is called" << '\n';
}
