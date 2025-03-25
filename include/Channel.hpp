/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:25 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/25 17:33:18 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_set>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"

class Channel {
  public:
    explicit Channel(std::string name, std::string topic,
                     const std::shared_ptr<Client> &client);

    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;

    Channel(Channel &&rhs) noexcept;
    Channel &operator=(Channel &&rhs) noexcept;

    ~Channel();

  public:
    enum Mode : std::uint8_t {
        INVITE_ONLY = 1 << 0,
        TOPIC_PROTECTED = 1 << 1,
        PASSWORD_PROTECTED = 1 << 2,
        USER_LIMIT = 1 << 3
    };

  public:
    IRCCode addUser(const std::string &password,
                    const std::shared_ptr<Client> &user);
    IRCCode removeUser(const std::shared_ptr<Client> &client);
    IRCCode kickUser(const std::shared_ptr<Client> &target,
                     const std::shared_ptr<Client> &client);
    IRCCode inviteUser(const std::shared_ptr<Client> &user,
                       const std::shared_ptr<Client> &client);

  public:
    IRCCode setMode(Mode mode, bool state,
                    const std::shared_ptr<Client> &client);
    IRCCode setPassword(const std::string &password,
                        const std::shared_ptr<Client> &client);
    IRCCode setUserLimit(size_t limit, const std::shared_ptr<Client> &client);
    IRCCode setTopic(const std::string &topic,
                     const std::shared_ptr<Client> &client);

  public:
    void addOperator(const std::shared_ptr<Client> &user);
    void removeOperator(const std::shared_ptr<Client> &user);

  public:
    const std::string &getName() const noexcept;
    const std::string &getTopic() const noexcept;
    size_t activeUsers() const noexcept;

  public:
    void broadcast(const std::string &senderPrefix,
                   const std::string &message) const;
    std::string getUserList() const noexcept;

  private:
    bool _hasPassword() const noexcept;
    bool _checkPassword(const std::string &password) const noexcept;
    bool _hasUserLimit() const noexcept;
    bool _hasInvite() const noexcept;

  private:

  private:
    bool _isOperator(const std::shared_ptr<Client> &user) const noexcept;
    bool _userOnChannel(const std::shared_ptr<Client> &user);

  private:
    IRCCode _addUser(const std::shared_ptr<Client> &user);

  private:
    std::string _name;
    std::string _topic;
    std::string _password;
    size_t _userLimit;
    uint8_t _modes;

  private:
    std::unordered_set<std::shared_ptr<Client>> _users;
    std::unordered_set<std::shared_ptr<Client>> _operators;
};

#endif // CHANNEL_HPP
