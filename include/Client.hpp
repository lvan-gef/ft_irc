/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/19 18:05:37 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/04 20:29:49 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <bitset>
#include <ctime>
#include <string>

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

  public:
    void setIP(const std::string &ip) noexcept;
    std::string getIP() const noexcept;

  public:
    void appendToBuffer(const std::string &data) noexcept;
    std::string getAndClearBuffer();
    bool hasCompleteMessage() const noexcept;

  private:
    FileDescriptors _fd;
    std::string _username;
    std::string _nickname;
    std::string _partial_buffer;
    std::string _ip;
    epoll_event _event;
    time_t _last_seen;
    std::bitset<3> _registered; // user, nick, pass
};

#endif // !CLIENT_HPP
