#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <string>

#include <netinet/in.h>
#include <sys/epoll.h>

#include "Client.hpp"

enum Ranges : std::uint16_t {
    LOWEST_PORT = 1024,
    HIGHEST_PORT = 65535,
    MAX_CLIENTS = 1024
};

class Server {
  public:
    Server(const std::string &port, std::string &password);

    Server(const Server &rhs);
    Server &operator=(const Server &rhs);

    Server(Server &&rhs) noexcept;
    Server &operator=(Server &&rhs) noexcept;

    ~Server();

  private:
    bool _init() noexcept;
    void _run();
    int _setNonBlocking(int fd);

  private:
    std::uint16_t _port;
    std::string _password;

  private:
    int _server_fd;
    int _epoll_fd;
    struct epoll_event _events[MAX_CLIENTS];
    /*std::array<struct epoll_event, MAX_CLIENTS> _events;*/
};

#endif // !SERVER_HPP
