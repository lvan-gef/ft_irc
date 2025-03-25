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
#include <unordered_set>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"

class Channel {
  public:
    explicit Channel(const std::string &name, const std::string &topic,
                     const std::shared_ptr<Client> &founder);

    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;

    Channel(Channel &&rhs) noexcept;
    Channel &operator=(Channel &&rhs) noexcept;

    ~Channel();

  public:
    enum Mode {
        INVITE_ONLY = 1 << 0,
        TOPIC_PROTECTED = 1 << 1,
        PASSWORD_PROTECTED = 1 << 2,
        USER_LIMIT = 1 << 3
    };

  public:
    IRCCode addUser(const std::string &password,
                    const std::shared_ptr<Client> &client);
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
    void addOperator(const std::shared_ptr<Client> &client);
    void removeOperator(const std::shared_ptr<Client> &client);

  public:
    const std::string &getName() const noexcept;
    const std::string &getTopic() const noexcept;
    size_t getUserCount() const noexcept;
    bool isOperator(const std::shared_ptr<Client> &client) const noexcept;
    bool hasPassword() const noexcept;

  public:
    void broadcast(const std::string &message,
                   const std::string &senderPrefix = "") const;

  private:
    bool checkPassword(const std::string &attempt) const noexcept;
    void sendJoinNotifications(const std::shared_ptr<Client> &client) const;
    std::string getUserList() const noexcept;

  private:
    std::string name_;
    std::string topic_;
    std::string password_;
    size_t userLimit_;
    uint8_t modes_;

  private:
    std::unordered_set<std::shared_ptr<Client>> users_;
    std::unordered_set<std::shared_ptr<Client>> operators_;
};

#endif // CHANNEL_HPP
