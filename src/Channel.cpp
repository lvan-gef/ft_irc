/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:09 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/25 21:57:14 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include "../include/Channel.hpp"
#include "../include/Enums.hpp"
#include "../include/Optional.hpp"
#include "../include/utils.hpp"

Channel::Channel(std::string name, std::string topic,
                 const std::shared_ptr<Client> &client)
    : _name(std::move(name)), _topic(std::move(topic)), _password(""),
      _userLimit(static_cast<size_t>(Defaults::USERLIMIT)), _modes(0), _users{},
      _operators{} {
    std::cout << "Default constructor called for Channel" << '\n';
    addUser(_password, client);
    addOperator(client);
}

Channel::Channel(Channel &&rhs) noexcept
    : _name(std::move(rhs._name)), _topic(std::move(rhs._topic)),
      _password(std::move(rhs._password)), _userLimit(rhs._userLimit),
      _modes(rhs._modes), _users(std::move(rhs._users)),
      _operators(std::move(rhs._operators)) {
    std::cout << "Default move constructor is called for Channel" << '\n';
}

Channel &Channel::operator=(Channel &&rhs) noexcept {
    if (this != &rhs) {
        _name = std::move(rhs._name);
        _topic = std::move(rhs._topic);
        _password = std::move(rhs._password);
        _userLimit = rhs._userLimit;
        _modes = rhs._modes;
        _users = std::move(rhs._users);
        _operators = std::move(rhs._operators);
    }

    std::cout << "Move assigment operator is called for Channel" << '\n';
    return *this;
}

Channel::~Channel() {
    std::cout << "Destructor is called for Channel: '" << _name << '\n';
}

IRCCode Channel::addUser(const std::string &password,
                         const std::shared_ptr<Client> &user) {

    if (_hasInvite() == true) {
        return IRCCode::INVITEONLYCHAN;
    }

    if (_hasPassword() == true) {
        if (_checkPassword(password) != true) {
            return IRCCode::BADCHANNELKEY;
        }
    }

    return _addUser(user);
}

IRCCode Channel::removeUser(const std::shared_ptr<Client> &user) {
    auto it = std::find(_users.begin(), _users.end(), user);

    if (it != _users.end()) {
        broadcast(user->getFullID(), "PART " + getName());
        _users.erase(user);

        return IRCCode::SUCCES;
    }

    return IRCCode::USERNOTINCHANNEL;
}

IRCCode Channel::kickUser(const std::shared_ptr<Client> &target,
                          const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    if (target == client) {
        return IRCCode::UNKNOWNCOMMAND;
    }

    if (_userOnChannel(target)) {
        broadcast(client->getFullID(),
                  "KICK " + getName() + " " + target->getNickname() + " :Bye");
        return removeUser(target);
    }

    return IRCCode::USERNOTINCHANNEL;
}

IRCCode Channel::inviteUser(const std::shared_ptr<Client> &user,
                            const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    return _addUser(user);
}

IRCCode Channel::setMode(ChannelMode mode, bool state, const std::string &value,
                         const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    switch (mode) {
        case ChannelMode::INVITE_ONLY:
            if (state) {
                _modes |= ChannelMode::INVITE_ONLY;
            } else {
                _modes &= ~ChannelMode::INVITE_ONLY;
            }
            break;
        case ChannelMode::TOPIC_PROTECTED:
            if (state) {
                _modes |= ChannelMode::TOPIC_PROTECTED;
            } else {
                _modes &= ~ChannelMode::TOPIC_PROTECTED;
            }
            break;
        case ChannelMode::PASSWORD_PROTECTED:
            if (state) {
                _modes |= ChannelMode::PASSWORD_PROTECTED;
                return setPassword(value, client);
            } else {
                _modes &= ~ChannelMode::PASSWORD_PROTECTED;
                return setPassword("", client);
            }
        case ChannelMode::USER_LIMIT:
            if (state) {
                _modes |= ChannelMode::USER_LIMIT;
                return setUserLimit(toSizeT(value), client);
            } else {
                _modes &= ~ChannelMode::USER_LIMIT;
                return setUserLimit(static_cast<size_t>(Defaults::USERLIMIT),
                                    client);
            }
        case ChannelMode::OPERATOR: {
            Optional<std::shared_ptr<Client>> _user = _nick_to_client(value);
            if (_user.has_value() != true) {
                return IRCCode::USERNOTINCHANNEL;
            }

            const std::shared_ptr<Client> user = std::move(_user.get_value());
            if (state) {
                addOperator(user);
            } else {
                removeOperator(user);
            }
            break;
        }
        default:
            return IRCCode::UNKNOWMODE;
    }

    return IRCCode::SUCCES;
}

IRCCode Channel::setPassword(const std::string &password,
                             const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    _password = password;
    return IRCCode::SUCCES;
}

IRCCode Channel::setUserLimit(size_t limit,
                              const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    _userLimit = limit;
    return IRCCode::SUCCES;
}

IRCCode Channel::setTopic(const std::string &topic,
                          const std::shared_ptr<Client> &client) {
    if (_hasTopic() && isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    _topic = topic;
    broadcast(client->getFullID(), "TOPIC " + getName() + " :" + _topic);
    return IRCCode::SUCCES;
}

Optional<std::shared_ptr<Client>>
Channel::addOperator(const std::shared_ptr<Client> &user) {
    auto it = std::find(_operators.begin(), _operators.end(), user);
    Optional<std::shared_ptr<Client>> client;

    if (it == _operators.end()) {
        _operators.emplace(user);
        client.set_value(user);
    }

    return client;
}

Optional<std::shared_ptr<Client>>
Channel::removeOperator(const std::shared_ptr<Client> &user) {
    auto it = std::find(_operators.begin(), _operators.end(), user);
    Optional<std::shared_ptr<Client>> client;

    if (it != _operators.end()) {
        _operators.erase(user);

        if (!_users.empty() && _operators.empty()) {
            std::shared_ptr<Client> newOperator = *_users.begin();
            addOperator(newOperator);
            client.set_value(newOperator);
        }
    }

    return client;
}

const std::string &Channel::getName() const noexcept {
    return _name;
}

const std::string &Channel::getTopic() const noexcept {
    return _topic;
}

std::size_t Channel::getActiveUsers() const noexcept {
    return _users.size();
}

std::string Channel::getModes() const noexcept {
    std::string modes = "+";

    if (_hasInvite() == true) {
        modes += "i";
    }

    if (_hasTopic() == true) {
        modes += "t";
    }

    if (_hasPassword() == true) {
        modes += "k";
    }

    if (_hasUserLimit() == true) {
        modes += "l";
    }

    return modes;
}

std::string Channel::getModesValues() const noexcept {
    std::string values = "";

    if (_hasPassword() == true) {
        values += (" " + _password);
    }

    if (_hasUserLimit() == true) {
        values += (" " + std::to_string(_userLimit));
    }

    return values;
}

void Channel::broadcast(const std::string &senderPrefix,
                        const std::string &message) const {
    for (const std::shared_ptr<Client> &user : _users) {
        if (message.find("PRIVMSG") != std::string::npos &&
            senderPrefix.find(user->getFullID()) != std::string::npos) {
            continue;
        }

        user->appendMessageToQue(
            formatMessage(":", senderPrefix, " ", message));
    }
}

std::string Channel::getUserList() const noexcept {
    std::string userList;
    for (const std::shared_ptr<Client> &user : _users) {
        userList += (isOperator(user) ? "@" : "") + user->getNickname() + " ";
    }

    return userList;
}

bool Channel::_hasPassword() const noexcept {
    return _modes & ChannelMode::PASSWORD_PROTECTED;
}

bool Channel::_checkPassword(const std::string &password) const noexcept {
    return _password == password;
}

bool Channel::_hasUserLimit() const noexcept {
    return _modes & ChannelMode::USER_LIMIT;
}

bool Channel::_hasInvite() const noexcept {
    return _modes & ChannelMode::INVITE_ONLY;
}

bool Channel::_hasTopic() const noexcept {
    return _modes & ChannelMode::TOPIC_PROTECTED;
}

bool Channel::isOperator(const std::shared_ptr<Client> &user) const noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), user);

    if (it != _operators.end()) {
        return true;
    }

    return false;
}

bool Channel::_userOnChannel(const std::shared_ptr<Client> &user) {
    auto it = std::find(_users.begin(), _users.end(), user);

    if (it == _users.end()) {
        return false;
    }

    return true;
}

Optional<std::shared_ptr<Client>>
Channel::_nick_to_client(const std::string &nickname) {
    Optional<std::shared_ptr<Client>> client;

    for (const std::shared_ptr<Client> &user : _users) {
        if (user->getNickname() == nickname) {
            client.set_value(user);
            break;
        }
    }

    return client;
}

IRCCode Channel::_addUser(const std::shared_ptr<Client> &user) {
    if (_userOnChannel(user) == true) {
        return IRCCode::USERONCHANNEL;
    }

    if (_hasUserLimit() && getActiveUsers() >= _userLimit) {
        return IRCCode::CHANNELISFULL;
    }

    _users.emplace(user);
    broadcast(user->getFullID(), "JOIN " + _name);

    return IRCCode::SUCCES;
}
