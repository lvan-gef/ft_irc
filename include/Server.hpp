#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server {
  public:
    Server(const char *port, const char *password);

    Server(const Server &rhs);
    Server &operator=(const Server &rhs);

    Server(Server &&rhs) noexcept;
    Server &operator=(Server &&rhs) noexcept;

    ~Server();
  public:
    void initServer();

  private:
    int _port;
    std::string _password;  // maybe make it a hash
};

#endif // !SERVER_HPP
