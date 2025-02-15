#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <string>
#include <sys/epoll.h>

#define LOWEST_PORT 1024
#define HIGHEST_PORT 65535
#define MAX_CLIENTS 1024

class Server {
  public:
    Server(const char *port, const char *password);

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
    uint16_t _port;
    std::string _password;

  private:
    int _server_fd;
    int _epoll_fd;
    struct epoll_event _events[MAX_CLIENTS];
};

#endif // !SERVER_HPP
