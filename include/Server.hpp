#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <string>

#define LOWEST_PORT 1024
#define HIGHEST_PORT 65535

class Server {
  public:
    Server(const char *port, const char *password);

    Server(const Server &rhs);
    Server &operator=(const Server &rhs);

    Server(Server &&rhs) noexcept;
    Server &operator=(Server &&rhs) noexcept;

    ~Server();
  public:
    bool startServer();

  private:
    int _port;
    std::string _password;
    int _socket;
    struct sockaddr_in _sockaddr;
    socklen_t _addrlen;
};

#endif // !SERVER_HPP
