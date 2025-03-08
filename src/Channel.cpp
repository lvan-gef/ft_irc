#include "../include/Channel.hpp"
#include <iostream>

Channel::Channel() {
    std::cout << "Default constructor for Channel is called" << '\n';
}

Channel::Channel(const Channel & /*rhs*/) {
    std::cout << "Default copy constructor for Channel is called" << '\n';
}

Channel &Channel::operator=(const Channel &rhs) {
    std::cout << "Copy assigment constructor for Channel is called" << '\n';
    if (this != &rhs) {
    }

    return *this;
}

Channel::Channel(Channel && /*rhs*/) noexcept {
    std::cout << "Default move constructor for Channel is called" << '\n';
}

Channel &Channel::operator=(Channel &&rhs) noexcept {
    std::cout << "Move assigment constructor for Channel is called" << '\n';
    if (this != &rhs) {
    }

    return *this;
}

Channel::~Channel() {
    std::cout << "Default destructor for Channel is called" << '\n';
}
