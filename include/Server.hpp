#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include <sys/socket.h>

#include "./Client.hpp"

enum Ranges : std::uint16_t {
    LOWEST_PORT = 1024,
    HIGHEST_PORT = 65535,
    INIT_EVENTS_SIZE = 10,
    MAX_CONNECTIONS = SOMAXCONN,
    READ_SIZE = 1024,
    INTERVAL = 1000
};

class Server {
  public:
    explicit Server(const std::string &port, std::string &password);

    Server(const Server &rhs);
    Server &operator=(const Server &rhs);

    Server(Server &&rhs) noexcept;
    Server &operator=(Server &&rhs) noexcept;

    ~Server();

  public:
    bool init() noexcept;
    bool run() noexcept;

  public:
    class ServerException : public std::exception {
      public:
        const char *what() const noexcept override;
    };

  private:
    bool _init() noexcept;
    void _run();
    void _shutdown() noexcept;
    int _setNonBlocking(int fd) noexcept;

  private:
    void _newConnection() noexcept;
    void _clientMessage(int fd) noexcept;
    void _removeClient(const std::shared_ptr<Client> &client) noexcept;
    void _processMessage(const std::shared_ptr<Client> &client) noexcept;

  private:
    std::uint16_t _port;
    std::string _password;

  private:
    int _server_fd;
    int _epoll_fd;
    size_t _connections;
    std::unordered_map<int, std::shared_ptr<Client>> _fd_to_client;
    std::unordered_map<std::string, std::shared_ptr<Client>> _nick_to_client;
};

#endif // !SERVER_HPP
