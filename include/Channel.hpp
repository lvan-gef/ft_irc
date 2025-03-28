/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.hpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:25 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/25 18:07:09 by lvan-gef      ########   odam.nl         */
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
#include "Optional.hpp"

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
    IRCCode addUser(const std::string &password,
                    const std::shared_ptr<Client> &user);
    IRCCode removeUser(const std::shared_ptr<Client> &user);
    IRCCode kickUser(const std::shared_ptr<Client> &target,
                     const std::shared_ptr<Client> &client);
    IRCCode inviteUser(const std::shared_ptr<Client> &user,
                       const std::shared_ptr<Client> &client);

  public:
    IRCCode setMode(ChannelMode mode, bool state, const std::string &value,
                    const std::shared_ptr<Client> &client);
    IRCCode setPassword(const std::string &password,
                        const std::shared_ptr<Client> &client);
    IRCCode setUserLimit(size_t limit, const std::shared_ptr<Client> &client);
    IRCCode setTopic(const std::string &topic,
                     const std::shared_ptr<Client> &client);

  public:
    Optional<std::shared_ptr<Client>>
    addOperator(const std::shared_ptr<Client> &user);
    Optional<std::shared_ptr<Client>>
    removeOperator(const std::shared_ptr<Client> &user);

  public:
    const std::string &getName() const noexcept;
    const std::string &getTopic() const noexcept;
    std::size_t getActiveUsers() const noexcept;
    std::string getModes() const noexcept;
    std::string getModesValues() const noexcept;

  public:
    void broadcast(const std::string &senderPrefix,
                   const std::string &message) const;
    std::string getUserList() const noexcept;

  public:
    bool isOperator(const std::shared_ptr<Client> &user) const noexcept;

  private:
    bool _hasPassword() const noexcept;
    bool _checkPassword(const std::string &password) const noexcept;
    bool _hasUserLimit() const noexcept;
    bool _hasInvite() const noexcept;
    bool _hasTopic() const noexcept;

  private:
    bool _userOnChannel(const std::shared_ptr<Client> &user);
    Optional<std::shared_ptr<Client>>
    _nick_to_client(const std::string &nickname);

  private:
    IRCCode _addUser(const std::shared_ptr<Client> &user);

  private:
    std::string _name;
    std::string _topic;
    std::string _password;
    size_t _userLimit;
    std::uint8_t _modes;

  private:
    std::unordered_set<std::shared_ptr<Client>> _users;
    std::unordered_set<std::shared_ptr<Client>> _operators;
};

#endif // CHANNEL_HPP
