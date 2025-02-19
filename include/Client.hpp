#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <ctime>
#include <string>

#include <sys/epoll.h>

class Client {
  public:
    explicit Client(int fd);

    Client(const Client &rhs) = delete;
    Client &operator=(const Client &rhs) = delete;

    Client(Client &&rhs) noexcept;
    Client &operator=(Client &&rhs) noexcept;

    ~Client();

  public:
    int getFD() const noexcept;
    const std::string &getUsername() const noexcept;
    const std::string &getNickname() const noexcept;
    epoll_event &getEvent() noexcept;
    bool isRegistered() const noexcept;
    time_t getLastSeen() const noexcept;

  public:
    void setUsername(const std::string &username) noexcept;
    void setNickname(const std::string &nickname) noexcept;
    void setRegistered(bool registered) noexcept;
    void updatedLastSeen() noexcept;

  public:
    void appendToBuffer(const std::string &data) noexcept;
    std::string getAndClearBuffer() noexcept;
    bool hasCompleteMessage() const noexcept;

  private:
    int _fd;
    std::string _username;
    std::string _nickname;
    std::string _partial_buffer;
    epoll_event _event;
    time_t _last_seen;
    bool _registered;
};

#endif // !CLIENT_HPP
