#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/Server.hpp"
#include "../include/utils.hpp"

Server::Server(const char *port, const char *password)
    : _port(toUint16(port)), _password(password), _server_fd(-1),
      _epoll_fd(-1) {
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
      _epoll_fd(rhs._epoll_fd) {
}

Server &Server::operator=(const Server &rhs) {
    if (this != &rhs) {
        _port = rhs._port;
        _password = rhs._password;
        _server_fd = rhs._server_fd;
        _epoll_fd = rhs._epoll_fd;
    }

    return *this;
}

Server::Server(Server &&rhs) noexcept
    : _port(rhs._port), _password(std::move(rhs._password)),
      _server_fd(rhs._server_fd), _epoll_fd(rhs._epoll_fd) {
}

Server &Server::operator=(Server &&rhs) noexcept {
    if (this != &rhs) {
        _port = rhs._port;
        _password = std::move(rhs._password);
        _server_fd = rhs._server_fd;
        _epoll_fd = rhs._epoll_fd;
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

    if (0 > _setNonBlocking(_server_fd)) {
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

    std::cout << "Server is running on: " << _port << '\n';
    return true;
}

void Server::_run() {
    while (true) {
        int events = epoll_wait(_epoll_fd, static_cast<epoll_event *>(_events),
                                MAX_CLIENTS, -1);
        if (0 > events) {
            std::cerr << "Epoll failed to wait: " << strerror(errno) << '\n';
            throw std::system_error();
        }

        for (int index = 0; index < events; ++index) {
            if (_events[index].data.fd == _server_fd) {
                struct sockaddr_in clientAddr{};
                socklen_t clientLen = sizeof(clientAddr);
                int clientFD = accept(
                    _server_fd,
                    (struct // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
                     sockaddr *)&clientAddr,
                    &clientLen);
                if (0 > clientFD) {
                    std::cerr << "Failed to accept client: " << strerror(errno)
                              << '\n';
                    throw std::system_error();
                }

                if (0 > _setNonBlocking(clientFD)) {
                    close(clientFD);
                    continue;
                }

                struct epoll_event ev{};
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = clientFD;
                if (0 > epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientFD, &ev)) {
                    std::cerr << "Epoll add failed: " << strerror(errno)
                              << '\n';
                    close(clientFD);
                    continue;
                }

                std::cout << "New client with fd: " << clientFD << '\n';
            } else {
                char buffer[1024];
                ssize_t readBytes = read(_events[index].data.fd, buffer, sizeof(buffer));

                if (readBytes <= 0) {
                    std::cout << "fd: " << _events[index].data.fd << " Disconnected" << '\n';
                    close(_events[index].data.fd);
                    continue;
                }

                buffer[readBytes] = '\0';
                std::cout << "Recieved from fd: " << _events[index].data.fd << ": " << buffer << '\n';
            }
        }
    }
}

int Server::_setNonBlocking(int fd) {
    int returnCode = fcntl(fd, // NOLINT(cppcoreguidelines-pro-type-vararg)
                           F_SETFL, O_NONBLOCK);
    if (0 > returnCode) {
        std::cerr << "Failed to set to non-blocking mode: " << strerror(errno)
                  << '\n';
        return false;
    }

    return returnCode;
}
