#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <bitset>
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

    ~Channel() = default;

  public:
    bool addUser(const std::string &password,
                 const std::shared_ptr<Client> &user);
    void removeUser(const std::shared_ptr<Client> &user,
                    const std::string &reason);
    void kickUser(const std::shared_ptr<Client> &target,
                  const std::shared_ptr<Client> &client,
                  const std::string &reason);
    void inviteUser(const std::shared_ptr<Client> &user,
                    const std::shared_ptr<Client> &client);

  public:
    void setMode(ChannelMode mode, bool state, const std::string &value,
                 const std::shared_ptr<Client> &client);
    void setPassword(const std::string &password,
                     const std::shared_ptr<Client> &client);
    void setUserLimit(size_t limit, const std::shared_ptr<Client> &client);
    void setTopic(const std::string &topic,
                  const std::shared_ptr<Client> &client);

  public:
    void addOperator(const std::shared_ptr<Client> &user);
    void removeOperator(const std::shared_ptr<Client> &user);

  public:
    const std::string &getName() const noexcept;
    const std::string &getTopic() const noexcept;
    std::size_t getActiveUsers() const noexcept;
    std::string getChannelModes() const noexcept;
    std::string getChannelModesValues() const noexcept;
    bool userOnChannel(const std::shared_ptr<Client> &user) const noexcept;

  public:
    void broadcast(IRCCode code, const std::string &senderPrefix,
                   const std::string &message) const;
    std::string getUserList() const noexcept;

  public:
    bool isOperator(const std::shared_ptr<Client> &user) const noexcept;

  public:
    bool hasInvite() const noexcept;
    bool isInvited(const std::shared_ptr<Client> &user) const noexcept;
    void removeFromInvited(const std::shared_ptr<Client> &user) noexcept;

  private:
    bool _hasPassword() const noexcept;
    bool _checkPassword(const std::string &password) const noexcept;
    bool _hasUserLimit() const noexcept;
    bool _hasTopic() const noexcept;

  private:
    bool _addUser(const std::shared_ptr<Client> &user);

  private:
    std::string _name;
    std::string _topic;
    std::string _password;
    size_t _userLimit;
    std::bitset<5> _modes; // invite, topic, password, operator, userlimit

  private:
    std::unordered_set<std::shared_ptr<Client>> _users;
    std::unordered_set<std::shared_ptr<Client>> _operators;
    std::vector<std::shared_ptr<Client>> _invites;
};

#endif // CHANNEL_HPP
