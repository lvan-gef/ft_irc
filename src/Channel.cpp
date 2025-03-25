/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:09 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/25 17:35:38 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"
#include "Enums.hpp"
#include "utils.hpp"
#include <algorithm>
#include <iostream>

Channel::Channel(std::string name, std::string topic,
                 const std::shared_ptr<Client> &client)
    : _name(std::move(name)), _topic(std::move(topic)), _password(""),
      _userLimit(Defaults::USERLIMIT), _modes(0), _users{}, _operators{} {
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

IRCCode Channel::setTopic(const std::string &topic,
                          const std::shared_ptr<Client> &client) {
    // check if is operator

    (void)client;
    _topic = topic;
    return IRCCode::SUCCES;
}

void Channel::addOperator(const std::shared_ptr<Client> &user) {
    auto it = std::find(_operators.begin(), _operators.end(), user);

    if (it == _operators.end()) {
        _operators.emplace(user);
    }
}

const std::string &Channel::getName() const noexcept {
    return _name;
}

const std::string &Channel::getTopic() const noexcept {
    return _topic;
}

std::size_t Channel::activeUsers() const noexcept {
    return _users.size();
}

void Channel::broadcast(const std::string &senderPrefix,
                        const std::string &message) const {
    for (const std::shared_ptr<Client> &client : _users) {
        client->appendMessageToQue(
            formatMessage(":", senderPrefix, " ", message));
    }
}

std::string Channel::getUserList() const noexcept {
    std::string userList;
    for (const std::shared_ptr<Client> &user : _users) {
        userList += (_isOperator(user) ? "@" : "") + user->getNickname() + " ";
    }

    return userList;
}

bool Channel::_hasPassword() const noexcept {
    return _modes & Mode::PASSWORD_PROTECTED;
}

bool Channel::_checkPassword(const std::string &password) const noexcept {
    return _password == password;
}

bool Channel::_hasUserLimit() const noexcept {
    return _modes & Mode::USER_LIMIT;
}

bool Channel::_hasInvite() const noexcept {
    return _modes & Mode::INVITE_ONLY;
}

bool Channel::_isOperator(const std::shared_ptr<Client> &user) const noexcept {
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

IRCCode Channel::_addUser(const std::shared_ptr<Client> &user) {
    if (_userOnChannel(user) == true) {
        return IRCCode::USERONCHANNEL;
    }

    if (_hasUserLimit() && activeUsers() >= _userLimit) {
        return IRCCode::CHANNELISFULL;
    }

    _users.emplace(user);
    broadcast(user->getFullID(), "JOIN " + _name);

    return IRCCode::SUCCES;
}

/*_broadcastMessage(" +o " + user->getNickname(), "MODE", _serverName);*/
