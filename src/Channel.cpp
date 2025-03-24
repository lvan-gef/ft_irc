/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:09 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/19 20:54:32 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>

#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Enums.hpp"
#include "../include/utils.hpp"

Channel::Channel(const std::string &serverName, const std::string &channelName,
                 const std::string &channelTopic,
                 const std::shared_ptr<Client> &client)
    : _serverName(serverName), _channelName(channelName), _topic(channelTopic),
      _password(""), _userLimit(USERLIMIT), _usersActive(1), _inviteOnly(false),
      _setTopicMode(false), _limitMode(false), _passwordProtected(false),
      _users{}, _operators{} {
    init(client);
}

Channel::Channel(Channel &&rhs) noexcept
    : _serverName(std::move(rhs._serverName)),
      _channelName(std::move(rhs._channelName)), _topic(std::move(rhs._topic)),
      _password(std::move(rhs._password)), _userLimit(rhs._userLimit),
      _usersActive(rhs._usersActive), _inviteOnly(rhs._inviteOnly),
      _setTopicMode(rhs._setTopicMode), _limitMode(rhs._limitMode),
      _passwordProtected(rhs._passwordProtected), _users(std::move(rhs._users)),
      _operators(std::move(rhs._operators)) {
}

Channel &Channel::operator=(Channel &&rhs) noexcept {
    if (this != &rhs) {
        _serverName = std::move(rhs._serverName);
        _channelName = std::move(rhs._channelName);
        _topic = std::move(rhs._topic);
        _password = std::move(rhs._password);
        _userLimit = rhs._userLimit;
        _usersActive = rhs._usersActive;
        _inviteOnly = rhs._inviteOnly;
        _setTopicMode = rhs._setTopicMode;
        _users = std::move(rhs._users);
        _operators = std::move(rhs._operators);
    }

    return *this;
}

Channel::~Channel() {
    std::cout << "Channel destructor is called: Remove channel: '"
              << _channelName << "' from the map" << '\n';
}

void Channel::init(const std::shared_ptr<Client> &client) {
    addOperator(client);
    addUser(_password, client);
}

IRCCode Channel::addUser(const std::string &password,
                         const std::shared_ptr<Client> &client) noexcept {

    if (_isInviteOnly()) {
        return IRCCode::INVITEONLYCHAN;
    }

    if (_passwordProtected) {
        if (_checkPassword(password) != true) {
            return IRCCode::BADCHANNELKEY;
        }
    }

    return _addUser(client);
}

IRCCode Channel::removeUser(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_users.begin(), _users.end(), client);

    if (it != _users.end()) {
        _broadcastMessage("", "PART", client->getFullID());
        _users.erase(client);
        _usersActive = _users.size();
        removeOperator(client);

        return IRCCode::SUCCES;
    }

    return IRCCode::USERNOTINCHANNEL;
}

void Channel::addOperator(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), client);

    if (it == _operators.end()) {
        _operators.emplace(client);
        _broadcastMessage(" +o " + client->getNickname(), "MODE", _serverName);
    }
}

void Channel::removeOperator(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), client);

    if (it != _operators.end()) {
        _operators.erase(client);
        _broadcastMessage(" -o " + client->getNickname(), "MODE", _serverName);

        if (!_users.empty() && _operators.empty()) {
            std::shared_ptr<Client> newOperator = *_users.begin();
            addOperator(newOperator);
        }
    }
}

IRCCode Channel::modeI(const std::string &state,
                       const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    if (state[0] == '-') {
        _inviteOnly = false;
        _broadcastMessage("-", "MODE", _serverName);
    } else {
        _broadcastMessage("+", "MODE", _serverName);
        _inviteOnly = true;
    }

    return IRCCode::SUCCES;
}

IRCCode Channel::modeT(const std::string &state,
                       const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    if (state[0] == '-') {
        _setTopicMode = false;
    } else {
        _setTopicMode = true;
    }

    return IRCCode::SUCCES;
}

IRCCode Channel::modeK(const std::string &state,
                       const std::shared_ptr<Client> &client,
                       const std::string &password) noexcept {
    if (_isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    if (state[0] == '-') {
        _passwordProtected = false;
    } else {
        _passwordProtected = true;
        _password = password;
    }

    return IRCCode::SUCCES;
}

IRCCode Channel::modeO(const std::string &state,
                       const std::shared_ptr<Client> &user,
                       const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    auto user_it = std::find(_users.begin(), _users.end(), user);
    if (user_it == _users.end()) {
        return IRCCode::NOSUCHNICK;
    }

    if (state[0] == '-') {
        removeOperator(user);
    } else {
        addOperator(user);
    }

    return IRCCode::SUCCES;
}

IRCCode Channel::modeL(const std::string &state, size_t limit,
                       const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    if (limit < 1) {
        // need to send a error code i guess;
        return IRCCode::UMODEUNKNOWNFLAG; // need to change
    }

    if (state[0] == '-') {
        _limitMode = false;
    } else {
        _userLimit = limit;
        _limitMode = true;
    }

    return IRCCode::SUCCES;
}

size_t Channel::usersActive() const noexcept {
    return _usersActive;
}

const std::string &Channel::channelName() const noexcept {
    return _channelName;
}

void Channel::sendMessage(const std::string &message,
                          const std::string &userID) noexcept {
    _broadcastMessage(message, "PRIVMSG", userID);
}

IRCCode Channel::kickUser(const std::shared_ptr<Client> &user,
                          const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client)) {
        if (user == client) {
            return IRCCode::UNKNOWNCOMMAND;
        }
        _broadcastMessage(" " + user->getNickname() + " :Bye", "KICK ",
                          client->getFullID());
        removeUser(user);
    }

    return IRCCode::CHANOPRIVSNEEDED;
}

IRCCode Channel::inviteUser(const std::shared_ptr<Client> &user,
                            const std::shared_ptr<Client> &client) noexcept {

    if (_isOperator(client) != true) {
        return IRCCode::CHANOPRIVSNEEDED;
    }

    return _addUser(user);
}

IRCCode Channel::setTopic(const std::string &topic,
                          const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client) || _setTopicMode) {
        _topic = topic.substr(1);
        _broadcastMessage(" :" + _topic, "TOPIC", client->getFullID());
        return IRCCode::SUCCES;
    }

    return IRCCode::CHANOPRIVSNEEDED;
}

const std::string Channel::getTopic() const noexcept {
    return _topic;
}

bool Channel::_checkPassword(const std::string &password) noexcept {
    return _password == password;
}

bool Channel::_isOperator(const std::shared_ptr<Client> &user) const noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), user);

    if (it != _operators.end()) {
        return true;
    }

    return false;
}

bool Channel::_isInviteOnly() const noexcept {
    return _inviteOnly;
}

void Channel::_broadcastMessage(const std::string &message,
                                const std::string &type,
                                const std::string &userID) const noexcept {
    for (const std::shared_ptr<Client> &client : _users) {
        if (type == "PRIVMSG" && userID == client->getFullID()) {
            continue;
        }

        client->appendMessageToQue(
            formatMessage(":", userID, " ", type, " ", _channelName, message));
    }
}

IRCCode Channel::_addUser(const std::shared_ptr<Client> &client) noexcept {
    std::string nickname = client->getNickname();
    auto it = std::find(_users.begin(), _users.end(), client);

    if (it != _users.end()) {
        return IRCCode::USERONCHANNEL;
    }

    if (_usersActive >= _userLimit && _limitMode) {
        return IRCCode::CHANNELISFULL;
    }

    _users.emplace(client);
    _usersActive = _users.size();

    std::string userList = _allUsersInChannel();
    _broadcastMessage("", "JOIN", client->getFullID());
    client->appendMessageToQue(formatMessage(
        ":", _serverName, " 332 ", nickname, " ", _channelName, " :", _topic));
    client->appendMessageToQue(formatMessage(":", _serverName, " 353 ",
                                             nickname, " = ", _channelName,
                                             " :", userList));
    client->appendMessageToQue(formatMessage(":", _serverName, " 366 ",
                                             nickname, " ", _channelName,
                                             " :End of /NAMES list"));
    return IRCCode::SUCCES;
}

std::string Channel::_allUsersInChannel() const noexcept {
    std::string userList;
    for (const std::shared_ptr<Client> &user : _users) {
        userList += (_isOperator(user) ? "@" : "") + user->getNickname() + " ";
    }

    return userList;
}
