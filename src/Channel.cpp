/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:09 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/28 14:25:50 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>

#include "../include/Channel.hpp"
#include "../include/Enums.hpp"
#include "../include/Utils.hpp"

Channel::Channel(std::string name, std::string topic,
                 const std::shared_ptr<Client> &client)
    : _name(std::move(name)), _topic(std::move(topic)), _password(""),
      _userLimit(getDefaultValue(Defaults::USERLIMIT)), _modes(0), _users{},
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

void Channel::addUser(const std::string &password,
                      const std::shared_ptr<Client> &user) {

    if (_hasInvite() == true) {
        return handleMsg(IRCCode::INVITEONLYCHAN, user, getName(), "");
    }

    if (_hasPassword() == true) {
        if (_checkPassword(password) != true) {
            return handleMsg(IRCCode::BADCHANNELKEY, user, getName(), "");
        }
    }

    return _addUser(user);
}

// it wrong i think
void Channel::removeUser(const std::shared_ptr<Client> &user) {
    auto it = std::find(_users.begin(), _users.end(), user);

    if (it == _users.end()) {
        return handleMsg(IRCCode::USERNOTINCHANNEL, user, getName(),
                         user->getNickname());
    }

    broadcast(user->getFullID(), getName(), IRCCode::PART);
    removeOperator(user);
    _users.erase(user);
}

void Channel::kickUser(const std::shared_ptr<Client> &target,
                       const std::shared_ptr<Client> &client,
                       const std::string &reason) {
    if (isOperator(client) != true) {
        return handleMsg(IRCCode::CHANOPRIVSNEEDED, client, getName(), "");
    }

    if (target == client) {
        return handleMsg(IRCCode::UNKNOWNCOMMAND, client, getName(),
                         "Can not kick your self");
    }

    if (_userOnChannel(target)) {
        broadcast(client->getFullID(),
                  getName() + " " + target->getNickname() + " :" + reason,
                  IRCCode::KICK);
        return removeUser(target);
    }

    return handleMsg(IRCCode::USERNOTINCHANNEL, client, getName(),
                     target->getNickname());
}

bool Channel::inviteUser(const std::shared_ptr<Client> &user,
                         const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        handleMsg(IRCCode::CHANOPRIVSNEEDED, client, getName(), "");
        return false;
    }

    _addUser(user);
    return true;
}

void Channel::setMode(ChannelMode mode, bool state, const std::string &value,
                      const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        return handleMsg(IRCCode::CHANOPRIVSNEEDED, client, getName(), "");
    }

    switch (mode) {
        case ChannelMode::INVITE_ONLY:
            state ? _modes.set(0) : _modes.reset(0);
            break;
        case ChannelMode::TOPIC_PROTECTED:
            state ? _modes.set(1) : _modes.reset(1);
            break;
        case ChannelMode::PASSWORD_PROTECTED:
            if (state) {
                _modes.set(2);
                return setPassword(value, client);
            } else {
                _modes.reset(2);
                return setPassword("", client);
            }
        case ChannelMode::OPERATOR:
            for (const std::shared_ptr<Client> &user : _users) {
                if (user->getNickname() == value) {
                    if (state) {
                        return addOperator(user);
                    } else {
                        return removeOperator(user);
                    }
                }
            }
            break;
        case ChannelMode::USER_LIMIT:
            if (state) {
                _modes.set(4);
                return setUserLimit(toSizeT(value), client);
            } else {
                _modes.reset(4);
                return setUserLimit(static_cast<size_t>(Defaults::USERLIMIT),
                                    client);
            }
        default:
            // need to see how to get the value out of it
            return handleMsg(IRCCode::UNKNOWMODE, client, "mode", "");
            /*return handleMsg(IRCCode::UNKNOWMODE, client, mode, "");*/
    }
}

void Channel::setPassword(const std::string &password,
                          const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        return handleMsg(IRCCode::CHANOPRIVSNEEDED, client, getName(), "");
    }

    _password = password;
}

void Channel::setUserLimit(size_t limit,
                           const std::shared_ptr<Client> &client) {
    if (isOperator(client) != true) {
        return handleMsg(IRCCode::CHANOPRIVSNEEDED, client, getName(), "");
    }

    _userLimit = limit;
}

void Channel::setTopic(const std::string &topic,
                       const std::shared_ptr<Client> &client) {
    if (_hasTopic() && isOperator(client) != true) {
        return handleMsg(IRCCode::CHANOPRIVSNEEDED, client, getName(), "");
    }

    _topic = topic;
    handleMsg(IRCCode::TOPICNOTICE, client, getName(), getTopic());
}

void Channel::addOperator(const std::shared_ptr<Client> &user) {
    auto it = std::find(_operators.begin(), _operators.end(), user);

    if (it == _operators.end()) {
        _operators.emplace(user);
    }

    return;
}

void Channel::removeOperator(const std::shared_ptr<Client> &user) {
    auto it = std::find(_operators.begin(), _operators.end(), user);

    if (it != _operators.end()) {
        _operators.erase(user);

        if (!_users.empty() && _operators.empty()) {
            addOperator(*_users.begin());
        }
    }
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
                        const std::string &message, IRCCode code) const {
    for (const std::shared_ptr<Client> &user : _users) {
        if (code == IRCCode::PRIVMSG &&
            senderPrefix.find(user->getFullID()) != std::string::npos) {
            continue;
        }

        handleMsg(code, user, senderPrefix, message);
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
    return _modes.test(2);
}

bool Channel::_checkPassword(const std::string &password) const noexcept {
    return _password == password;
}

bool Channel::_hasUserLimit() const noexcept {
    return _modes.test(4);
}

bool Channel::_hasInvite() const noexcept {
    return _modes.test(0);
}

bool Channel::_hasTopic() const noexcept {
    return _modes.test(1);
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

void Channel::_addUser(const std::shared_ptr<Client> &user) {
    if (_userOnChannel(user) == true) {
        return handleMsg(IRCCode::USERONCHANNEL, user, getName(), "");
    }

    if (_hasUserLimit() && getActiveUsers() >= _userLimit) {
        return handleMsg(IRCCode::CHANNELISFULL, user, getName(), "");
    }

    _users.emplace(user);
    broadcast(user->getFullID(), _name, IRCCode::JOIN);
}
