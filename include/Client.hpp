#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <bitset>
#include <ctime>
#include <queue>
#include <string>
#include <vector>

#include <sys/epoll.h>

#include "./EpollInterface.hpp"
#include "./FileDescriptors.hpp"

class Client {
  public:
    explicit Client(int fd);

    Client(const Client &rhs) = delete;
    Client &operator=(const Client &rhs) = delete;

    Client(Client &&rhs) noexcept;
    Client &operator=(Client &&rhs) noexcept;

    ~Client();

  public:
    void setEpollNotifier(EpollInterface *notifier);

  public:
    int getFD() const noexcept;
    epoll_event &getEvent() noexcept;
    bool isRegistered() const noexcept;

  public:
    void setUsername(const std::string &username) noexcept;
    void setRealname(const std::string &realname) noexcept;
    void setNickname(const std::string &nickname) noexcept;
    const std::string &getUsername() const noexcept;
    const std::string &getNickname() const noexcept;
    const std::string &getRealname() const noexcept;

  public:
    void setUsernameBit() noexcept;
    void setNicknameBit() noexcept;
    void setPasswordBit() noexcept;
    bool getUsernameBit() const noexcept;
    bool getNicknameBit() const noexcept;
    bool getPasswordBit() const noexcept;
    std::string getFullID() const noexcept;

  public:
    void setIP(const std::string &ip) noexcept;
    const std::string &getIP() const noexcept;

  public:
    void appendToBuffer(const std::string &data) noexcept;
    std::string getAndClearBuffer() noexcept;
    bool hasCompleteMessage() const noexcept;

  public:
    std::string getMessage() noexcept;
    void removeMessage() noexcept;
    bool haveMessagesToSend() noexcept;
    void appendMessageToQue(const std::string &msg) noexcept;
    void setDisconnect() noexcept;
    bool isDisconnect() const noexcept;

  public:
    void addChannel(const std::string &channelName) noexcept;
    void removeChannel(const std::string &channelName) noexcept;
    void removeAllChannels() noexcept;
    const std::vector<std::string> &allChannels() noexcept;

  public:
    void setOffset(size_t offset) noexcept;
    size_t getOffset() const noexcept;

  private:
    EpollInterface *_epollNotifier{};

  private:
    FileDescriptors _fd;
    std::string _username;
    std::string _nickname;
    std::string _ip;
    std::string _realname;

  private:
    std::string _partial_buffer;
    std::queue<std::string> _messages;
    size_t _offset{0};

  private:
    epoll_event _event;
    std::bitset<3> _registered{0}; // user, nick, pass
    bool _disconnect{false};

  private:
    std::vector<std::string> _channels;
};

#endif // !CLIENT_HPP
