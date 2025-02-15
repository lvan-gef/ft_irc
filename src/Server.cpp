#include <asm-generic/socket.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

#include "../include/Server.hpp"
#include "../include/utils.hpp"

Server::Server(const char *port, const char *password)
    : _port(toUint16(port)), _password(password), _server_fd(-1), _epoll_fd(-1),
      _events({}) {
    if (errno != 0) {
        throw std::invalid_argument("Invalid port");
    }

    if (_port < LOWEST_PORT || _port > HIGHEST_PORT) {
        throw std::range_error("Port is out of range");
    }

    if (_password.length() == 0) {
        throw std::invalid_argument("password can not be empty");
    }

    if (_init() != true) {
        throw std::system_error();
    }
    _run();
}

Server::Server(const Server &rhs)
    : _port(rhs._port), _password(rhs._password), _server_fd(rhs._server_fd),
      _epoll_fd(rhs._epoll_fd), _events(rhs._events) {
}

Server &Server::operator=(const Server &rhs) {
    if (this != &rhs) {
        _port = rhs._port;
        _password = rhs._password;
        _server_fd = rhs._server_fd;
        _epoll_fd = rhs._epoll_fd;
        _events = rhs._events;
    }

    return *this;
}

Server::Server(Server &&rhs) noexcept
    : _port(rhs._port), _password(std::move(rhs._password)),
      _server_fd(rhs._server_fd), _epoll_fd(rhs._epoll_fd),
      _events(rhs._events) {
}

Server &Server::operator=(Server &&rhs) noexcept {
    if (this != &rhs) {
        _port = rhs._port;
        _password = std::move(rhs._password);
        _server_fd = rhs._server_fd;
        _epoll_fd = rhs._epoll_fd;
        _events = rhs._events;
    }

    return *this;
}

Server::~Server() {
    if (_epoll_fd >= 0) {
        close(_epoll_fd);
    }

    if (_server_fd >= 0) {
        close(_server_fd);
    }
}

bool Server::_init() noexcept {
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > _server_fd) {
        std::cerr << "Failed to create a socket: " << strerror(errno) << '\n';
        return false;
    }

    int opt = 1;
    if (0 >
        setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed: " << strerror(errno) << '\n';
        return false;
    }

    if (0 > fcntl(_server_fd, // NOLINT(cppcoreguidelines-pro-type-vararg)
                  F_SETFL, O_NONBLOCK)) {
        std::cerr << "Failed to set to non-blocking mode: " << strerror(errno)
                  << '\n';
        return false;
    }

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);
    if (0 >
        bind(_server_fd,
             (struct sockaddr // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                  *)&address,
             sizeof(address))) {
        std::cerr << "Bind failed: " << strerror(errno) << '\n';
        return false;
    }

    if (0 > listen(_server_fd, SOMAXCONN)) {
        std::cerr << "Listen failed: " << strerror(errno) << '\n';
        return false;
    }

    _epoll_fd = epoll_create1(0);
    if (0 > _epoll_fd) {
        std::cerr << "Epoll create failed: " << strerror(errno) << '\n';
        return false;
    }

    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = _server_fd;
    if (0 > epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _server_fd, &ev)) {
        std::cerr << "Epoll add failed: " << strerror(errno) << '\n';
        return false;
    }

    return true;
}

void Server::_run() {
}
