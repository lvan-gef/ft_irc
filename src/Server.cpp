/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/19 17:48:48 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/04 20:29:49 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <atomic>
#include <csignal>
#include <cstring>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/utils.hpp"

static std::atomic<bool> g_running{true};

static void signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        g_running = false;
    }
}

Server::Server(const std::string &port, std::string &password)
    : _port(toUint16(port)), _password(std::move(password)),
      _serverName("codamirc.local"), _serverVersion("0.1.0"),
      _serverCreated("Mon Feb 19 2025 at 10:00:00 UTC"), _server_fd(-1),
      _epoll_fd(-1), _connections(0) {
    if (errno != 0) {
        throw std::invalid_argument("Invalid port");
    }

    if (_port < LOWEST_PORT || _port > HIGHEST_PORT) {
        throw std::range_error("Port is out of range");
    }

    if (_password.length() == 0) {
        throw std::invalid_argument("password can not be empty");
    }

    if (init() != true) {
        throw std::system_error();
    }

    run();
}

Server::Server(Server &&rhs) noexcept
    : _port(rhs._port), _password(std::move(rhs._password)),
      _serverName(std::move(rhs._serverName)),
      _serverVersion(std::move(rhs._serverVersion)),
      _serverCreated(std::move(rhs._serverCreated)), _server_fd(rhs._server_fd),
      _epoll_fd(rhs._epoll_fd), _connections(rhs._connections),
      _fd_to_client(std::move(rhs._fd_to_client)),
      _nick_to_client(std::move(rhs._nick_to_client)) {
}

Server &Server::operator=(Server &&rhs) noexcept {
    if (this != &rhs) {
        _port = rhs._port;
        _password = std::move(rhs._password);
        _serverName = std::move(rhs._serverName);
        _serverVersion = std::move(rhs._serverVersion);
        _serverCreated = std::move(rhs._serverCreated);
        _server_fd = rhs._server_fd;
        _epoll_fd = rhs._epoll_fd;
        _connections = rhs._connections;
        _fd_to_client = std::move(rhs._fd_to_client);
        _nick_to_client = std::move(rhs._nick_to_client);
    }

    return *this;
}

Server::~Server() {
    _shutdown();
}

bool Server::init() noexcept {
    if (_server_fd >= 0 || _epoll_fd >= 0) {
        std::cerr << "Server already initialized" << '\n';
        return false;
    }

    return _init();
}

bool Server::run() noexcept {
    if (0 > _server_fd || 0 > _epoll_fd) {
        std::cerr << "Server is not initialized. Call init() first then run()"
                  << '\n';
        return false;
    }

    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        std::cerr << "Failed to set up SGINT handler" << '\n';
        return false;
    }
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        std::cerr << "Failed to set up SIGTERM handler" << '\n';
        return false;
    }

    try {
        _run();
    } catch (const ServerException &e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
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

    std::cout << "Server is running on: " << _port << ". Press Ctrl+C to stop."
              << '\n';
    return true;
}

void Server::_run() {
    std::vector<epoll_event> events(INIT_EVENTS_SIZE);

    while (g_running) {
        if (events.capacity() != _connections &&
            _connections > INIT_EVENTS_SIZE) {
            events.resize(_connections);
        }

        int nfds =
            epoll_wait(_epoll_fd, static_cast<epoll_event *>(events.data()),
                       (int)events.size(), INTERVAL);

        if (0 > nfds) {
            if (errno == EINTR) {
                continue;
            }

            std::cerr << "Epoll wait failed: " << strerror(errno) << '\n';
            throw ServerException();
        }

        for (size_t index = 0; index < static_cast<size_t>(nfds); ++index) {
            const auto &event = events[index];

            if (event.data.fd == _server_fd) {
                _newConnection();
            } else if (event.events & EPOLLIN) {
                _clientMessage(event.data.fd);
            } else if (event.events & EPOLLOUT) {
                /*std::cout << "What we need to send back no idea how to get it"
                 * << '\n';*/
                /*_sendMessage(event.data.fd,*/
                /*             "What we need to send back no idea how to get
                 * it");*/
            } else if (event.events & (EPOLLRDHUP | EPOLLHUP)) {
                if (auto client = _fd_to_client[event.data.fd]) {
                    _removeClient(client);
                }
            } else {
                std::cout << "Unknow message from client: " << event.data.fd
                          << '\n';
            }
        }
    }

    _shutdown();
}

void Server::_shutdown() noexcept {
    std::cout << '\n' << "Shutting down server..." << '\n';
    for (const auto &pair : _fd_to_client) {
        _removeClient(pair.second);
    }

    _fd_to_client.clear();
    _nick_to_client.clear();

    if (_epoll_fd >= 0) {
        close(_epoll_fd);
        _epoll_fd = -1;
    }

    if (_server_fd >= 0) {
        close(_server_fd);
        _server_fd = -1;
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

    std::shared_ptr<Client> client = std::make_shared<Client>(clientFD);

    if (0 >
        epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientFD, &client->getEvent())) {
        std::cerr << "Failed to add client to epoll" << '\n';
        return;
    }

    _fd_to_client[clientFD] = client;
    _connections++;

    std::cout << "New client connected on fd " << clientFD << '\n';
}

void Server::_clientAccepted(const std::shared_ptr<Client> &client) noexcept {
    if (client->isRegistered() != true) {
        return;
    }

    std::string nick = "lvan-gef";
    std::string user = "lvan-gef";
    std::string ip = "127.0.0.1";

    _sendMessage(client->getFD(),
                 ":" + _serverName + " 001 " + nick +
                     " :Welcome to the Internet Relay Network " + nick + "!" +
                     user + "@" + ip + "\r\n");
    _sendMessage(client->getFD(), ":" + _serverName + " 002 " + nick +
                                      " :Your host is " + _serverName +
                                      ", running version " + _serverVersion +
                                      "\r\n");
    _sendMessage(client->getFD(), ":" + _serverName + " 003 " + nick +
                                      " :This server was created " +
                                      _serverCreated + "\r\n");
    _sendMessage(client->getFD(), ":" + _serverName + " 004 " + nick + " " +
                                      _serverName + " " + _serverVersion +
                                      " oOiws abiklmnopqrstv\r\n");
    _sendMessage(
        client->getFD(),
        ":" + _serverName + " 005 " + nick +
            " CHANTYPES=# PREFIX=(ov)@+ :are supported by this server\r\n");
    _sendMessage(client->getFD(), ":" + _serverName + " 375 " + nick + " :- " +
                                      _serverName +
                                      " Message of the Day -\r\n");
    _sendMessage(client->getFD(), ":" + _serverName + " 372 " + nick +
                                      " :- Welcome to my IRC server!\r\n");
    _sendMessage(client->getFD(), ":" + _serverName + " 376 " + nick +
                                      " :End of /MOTD command.\r\n");
}

void Server::_clientMessage(int fd) noexcept {
    std::shared_ptr<Client> client = _fd_to_client[fd];
    if (!client) {
        return;
    }

    char buffer[READ_SIZE] = {0};
    ssize_t bytes_read = recv(fd, buffer, READ_SIZE - 1, 0);
    if (0 > bytes_read) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
    }

    if (bytes_read == 0) {
        _removeClient(client);
        return;
    }

    std::cerr << buffer << '\n';
    client->updatedLastSeen();
    client->appendToBuffer(std::string(buffer, (size_t)bytes_read));

    _processMessage(client);
}

void Server::_removeClient(const std::shared_ptr<Client> &client) noexcept {
    if (!client) {
        return;
    }

    int fd = client->getFD();
    std::string nickname = client->getNickname();

    try {
        auto fd_it = _fd_to_client.find(fd);
        auto nick_it = _nick_to_client.find(nickname);

        if (fd_it != _fd_to_client.end() && fd_it->second == client) {
            _fd_to_client.erase(fd_it);
            epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
            _connections--;
        }

        if (nick_it != _nick_to_client.end() && nick_it->second == client) {
            _nick_to_client.erase(nick_it);
        }

        std::cout << "Client disconnected - FD: " << fd << " Nickname: '"
                  << nickname << "' - " << '\n';
    } catch (const std::exception &e) {
        std::cerr << "Error while removing client - FD: " << fd
                  << " Nickname: '" << nickname << "' - " << e.what() << '\n';
    }
}

void Server::_processMessage(const std::shared_ptr<Client> &client) noexcept {
    if (client->hasCompleteMessage() != true) {
        return;
    }

    std::string msg = client->getAndClearBuffer();

    std::vector<IRCMessage> clientsToken = parseIRCMessage(msg);
    for (const IRCMessage &token : clientsToken) {
        switch (token.type) {
            case IRCCommand::NICK:
                if (token.success) {
                    client->setNickname(token.params[0]);
                    _clientAccepted(client);
                } else {
                    _handleError(token, client);
                }
                break;
            case IRCCommand::USER:
                if (token.success) {
                    client->setUsername(token.params[0]);
                    _clientAccepted(client);
                } else {
                    _handleError(token, client);
                }
                break;
            case IRCCommand::PASS:
                if (token.success) {
                    client->setPasswordBit();
                    _clientAccepted(client);
                } else {
                    _handleError(token, client);
                }
                break;
            case IRCCommand::PRIVMSG:
                std::cerr << "Not impl yet PRIV" << '\n';
                break;
            case IRCCommand::JOIN:
                std::cerr << "Not impl yet JOIN" << '\n';
                token.print();
                break;
            case IRCCommand::PART:
                std::cerr << "Not impl yet PART" << '\n';
                break;
            case IRCCommand::QUIT:
                std::cerr << "Not impl yet QUIT" << '\n';
                break;
            case IRCCommand::PING:
                _sendMessage(client->getFD(), ":" + _serverName + " PONG " +
                                                  _serverName + " :" +
                                                  token.params[0] + "\r\n");
                break;
            case IRCCommand::UNKNOW:
                std::cerr << "--------------------------------------" << '\n';
                std::cerr << "Not impl yet UNKNOW" << '\n';
                token.print();
                std::cerr << "--------------------------------------" << '\n';
                break;
            default:
                std::cerr << "i need to handle it" << '\n';
        }
    }
}

void Server::_sendMessage(int fd, const std::string &msg) noexcept {
    send(fd, msg.c_str(), msg.length(), 0);
}
