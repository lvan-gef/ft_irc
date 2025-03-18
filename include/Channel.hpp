/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:25 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/17 21:29:57 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_set>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"

class Channel {
  public:
    explicit Channel(const std::string &serverName,
                     const std::string &channelName,
                     const std::string &channelTopic,
                     const std::shared_ptr<Client> &client);

    Channel(const Channel &rhs) = delete;
    Channel &operator=(const Channel &rhs) = delete;

    Channel(Channel &&rhs) noexcept;
    Channel &operator=(Channel &&rhs) noexcept;

    ~Channel();

  public:
    void init(const std::shared_ptr<Client> &client);

  public:
    IRCCodes addUser(const std::shared_ptr<Client> &client) noexcept;
    void removeUser(const std::shared_ptr<Client> &client) noexcept;

  public:
    void banUser(const std::shared_ptr<Client> &client) noexcept;
    void unbanUser(const std::shared_ptr<Client> &client) noexcept;
    bool isBannedUser(const std::shared_ptr<Client> &client) const noexcept;

  public:
    void addOperator(const std::shared_ptr<Client> &client) noexcept;
    void removeOperator(const std::shared_ptr<Client> &client) noexcept;
    bool isOperator(const std::shared_ptr<Client> &client) const noexcept;

  public:
    size_t usersActive() const noexcept;
    std::string channelName() const noexcept;
    std::string allUsersInChannel() const noexcept;

  public:
    void broadcastMessage(const std::string &message,
                          const std::string &fromUser) const noexcept;

  public:
    void setTopic(const std::string &topic) noexcept;
    bool inviteOnly() const noexcept;

  private:
    std::string _serverName;
    std::string _channelName;
    std::string _topic;
    size_t _userLimit;
    size_t _usersActive;

  private:
    bool _inviteOnly;

  private:
    std::unordered_set<std::shared_ptr<Client>> _users;
    std::unordered_set<std::shared_ptr<Client>> _banned;
    std::unordered_set<std::shared_ptr<Client>> _operators;
};

#endif // !CHANNEL_HPP
