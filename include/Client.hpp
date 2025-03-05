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

#include <ctime>
#include <string>

#include <sys/epoll.h>

#define ISREGISTERED 7

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
    void updatedLastSeen() noexcept;

  public:
    bool getUsernameBit() const noexcept;
    bool setNicknameBit() const noexcept;
    bool getPasswordBit() const noexcept;
    void setUsernameBit() noexcept;
    void setNicknameBit() noexcept;
    void setPasswordBit() noexcept;

  public:
    void appendToBuffer(const std::string &data) noexcept;
    std::string getAndClearBuffer();
    bool hasCompleteMessage() const noexcept;

  private:
    int _fd;
    std::string _username;
    std::string _nickname;
    std::string _partial_buffer;
    epoll_event _event;
    time_t _last_seen;
    unsigned int _registered;  // 111 user, nick, pass
};

#endif // !CLIENT_HPP
