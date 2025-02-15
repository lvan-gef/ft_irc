#ifndef CLIENT_HPP
#define CLIENT_HPP

class Client {
  public:
    Client();

    Client(const Client &rhs);
    Client &operator=(const Client &rhs);

    Client(Client &&rhs) noexcept;
    Client &operator=(Client &&rhs) noexcept;

    ~Client();
};

#endif // !CLIENT_HPP
