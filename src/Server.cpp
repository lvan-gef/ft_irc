#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <vector>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/Server.hpp"
#include "../include/utils.hpp"
#include "Client.hpp"

Server::Server(const std::string &port, std::string &password)
    : _port(toUint16(port)), _password(std::move(password)), _server_fd(-1),
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
    for (const auto &pair : _fd_to_client) {
        delete pair.second;
    }

    if (_epoll_fd >= 0) {
        close(_epoll_fd);
    }

    if (_server_fd >= 0) {
        close(_server_fd);
    }
}

const char *Server::ServerException::what() const noexcept {
    return "Internal server error";
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
    if (0 > bind(_server_fd, (struct sockaddr *)&address, sizeof(address))) {
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
    std::vector<epoll_event> events(INIT_EVENTS_SIZE);

    while (true) {
        int nfds =
            epoll_wait(_epoll_fd, static_cast<epoll_event *>(events.data()),
                       MAX_CONNECTIONS, -1);

        if (0 > nfds) {
            // maybe add check EINTR
            std::cerr << "Epoll wait failed: " << strerror(errno) << '\n';
            throw ServerException();
        }

        for (size_t index = 0; index < static_cast<size_t>(nfds); ++index) {
            const auto &event = events[index];

            if (event.data.fd == _server_fd) {
                _newConnection();
            } else {
                if (event.events & EPOLLIN) {
                    _clientMessage(event.data.fd);
                } else if (event.events & (EPOLLRDHUP | EPOLLHUP)) {
                    if (auto client = _fd_to_client[event.data.fd]) {
                        _removeClient(client);
                    }
                } else {
                    std::cout << "Iets anders" << '\n';
                }
            }
        }
    }
}

int Server::_setNonBlocking(int fd) noexcept {
    int returnCode = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (0 > returnCode) {
        std::cerr << "Failed to set to non-blocking mode: " << strerror(errno)
                  << '\n';
    }

    return returnCode;
}

void Server::_newConnection() noexcept {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);

    int clientFD = accept(_server_fd, reinterpret_cast<sockaddr *>(&clientAddr),
                          &clientLen);

    if (0 > clientFD) {
        std::cerr << "Accept failed: " << strerror(errno) << '\n';
        return;
    }

    if (0 > _setNonBlocking(clientFD)) {
        close(clientFD);
        return;
    }

    auto *client = new Client(clientFD);

    if (0 >
        epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientFD, &client->getEvent())) {
        std::cerr << "Failed to add client to epoll" << '\n';
        delete client;
        return;
    }

    _fd_to_client[clientFD] = client;

    std::cout << "New client connected on fd " << clientFD << '\n';
}

void Server::_clientMessage(int fd) noexcept {
    auto client = _fd_to_client[fd];
    if (!client) {
        return;
    }

    char buffer[READ_SIZE];
    ssize_t bytes_read = read(fd, buffer, READ_SIZE);
    if (0 > bytes_read) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
    }

    if (bytes_read == 0) {
        _removeClient(client);
        return;
    }

    client->updatedLastSeen();
    client->appendToBuffer(std::string(buffer, (size_t)bytes_read));

    if (client->hasCompleteMessage()) {
        _processMessage(client);
    }
}

void Server::_removeClient(Client *client) noexcept {
    if (!client) {
        return;
    }

    int fd = client->getFD();
    std::cout << "Client disconnected on fd: " << fd << '\n';

    _fd_to_client.erase(fd);
    if (!client->getNickname().empty()) {
        _nick_to_client.erase(client->getNickname());
    }

    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
    delete client;
}

void Server::_processMessage(Client *client) noexcept {
    std::string msg = client->getAndClearBuffer();

    std::cout << "Parse the message: " << msg << '\n';
}
