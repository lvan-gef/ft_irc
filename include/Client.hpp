/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/19 18:05:37 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/17 20:26:05 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <bitset>
#include <ctime>
#include <queue>
#include <string>
#include <vector>

#include <sys/epoll.h>

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
    int getFD() const noexcept;
    epoll_event &getEvent() noexcept;
    bool isRegistered() const noexcept;

  public:
    void setUsername(const std::string &username) noexcept;
    void setNickname(const std::string &nickname) noexcept;
    const std::string &getUsername() const noexcept;
    const std::string &getNickname() const noexcept;

  public:
    void updatedLastSeen() noexcept;
    time_t getLastSeen() const noexcept;

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
    std::string getAndClearBuffer();
    bool hasCompleteMessage() const noexcept;

  public:
    std::string getMessage();
    void removeMessage();
    bool haveMessagesToSend();
    void appendMessageToQue(const std::string &msg) noexcept;

  public:
    void addChannel(const std::string &channelName) noexcept;
    void removeChannel(const std::string &channelName) noexcept;
    void removeAllChannels() noexcept;
    std::vector<std::string> allChannels() noexcept;

  public:
    void setOffset(size_t offset) noexcept;
    size_t getOffset() const noexcept;

  private:
    FileDescriptors _fd;
    std::string _username;
    std::string _nickname;
    std::string _ip;

  private:
    std::string _partial_buffer;
    std::queue<std::string> _messages;
    size_t _offset;

  private:
    epoll_event _event;
    time_t _last_seen;
    std::bitset<3> _registered; // user, nick, pass

  private:
    std::vector<std::string> _channels;
};

#endif // !CLIENT_HPP
