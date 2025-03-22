/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:25 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/19 19:27:08 by lvan-gef      ########   odam.nl         */
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
    IRCCodes addUser(const std::string &password,
                     const std::shared_ptr<Client> &client) noexcept;
    IRCCodes removeUser(const std::shared_ptr<Client> &client) noexcept;

  public:
    void banUser(const std::shared_ptr<Client> &client) noexcept;
    void unbanUser(const std::shared_ptr<Client> &client) noexcept;

  public:
    void addOperator(const std::shared_ptr<Client> &client) noexcept;
    void removeOperator(const std::shared_ptr<Client> &client) noexcept;

  public:
    IRCCodes modeI(const std::string &state,
                   const std::shared_ptr<Client> &client) noexcept;
    IRCCodes modeT(const std::string &state,
                   const std::shared_ptr<Client> &client) noexcept;
    IRCCodes modeK(const std::string &state,
                   const std::shared_ptr<Client> &client,
                   const std::string &password) noexcept;

  public:
    size_t usersActive() const noexcept;
    const std::string &channelName() const noexcept;

  public:
    void sendMessage(const std::string &message,
                     const std::string &userID) noexcept;
    IRCCodes kickUser(const std::shared_ptr<Client> &user,
                      const std::shared_ptr<Client> &client) noexcept;
    IRCCodes inviteUser(const std::shared_ptr<Client> &user,
                        const std::shared_ptr<Client> &client) noexcept;

  public:
    IRCCodes setTopic(const std::string &topic,
                      const std::shared_ptr<Client> &client) noexcept;
    const std::string getTopic() const noexcept;

  private:
    bool _checkPassword(const std::string &password) noexcept;

  private:
    bool _isOperator(const std::shared_ptr<Client> &user) const noexcept;
    bool _isBannedUser(const std::shared_ptr<Client> &user) const noexcept;
    bool _isInviteOnly() const noexcept;
    void _broadcastMessage(const std::string &message, const std::string &type,
                           const std::string &userID) const noexcept;
    IRCCodes _addUser(const std::shared_ptr<Client> &client) noexcept;

  private:
    std::string _allUsersInChannel() const noexcept;

  private:
    std::string _serverName;
    std::string _channelName;
    std::string _topic;
    std::string _password;
    size_t _userLimit;
    size_t _usersActive;

  private:
    bool _inviteOnly;
    bool _setTopicMode;
    bool _passwordProtected;

  private:
    std::unordered_set<std::shared_ptr<Client>> _users;
    std::unordered_set<std::shared_ptr<Client>> _banned;
    std::unordered_set<std::shared_ptr<Client>> _operators;
};

#endif // !CHANNEL_HPP
