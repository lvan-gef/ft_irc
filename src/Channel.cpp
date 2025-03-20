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
      _userLimit(USERLIMIT), _usersActive(1), _inviteOnly(false), _users{},
      _banned{}, _operators{} {
    init(client);
}

Channel::Channel(Channel &&rhs) noexcept
    : _serverName(std::move(rhs._serverName)),
      _channelName(std::move(rhs._channelName)), _topic(std::move(rhs._topic)),
      _userLimit(rhs._userLimit), _usersActive(rhs._usersActive),
      _inviteOnly(false), _users(std::move(rhs._users)),
      _banned(std::move(rhs._banned)), _operators(std::move(rhs._operators)) {
}

Channel &Channel::operator=(Channel &&rhs) noexcept {
    if (this != &rhs) {
        _serverName = std::move(rhs._serverName);
        _channelName = std::move(rhs._channelName);
        _topic = std::move(rhs._topic);
        _userLimit = rhs._userLimit;
        _usersActive = rhs._usersActive;
        _users = std::move(rhs._users);
        _banned = std::move(rhs._banned);
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
    addUser(client);
}

IRCCodes Channel::addUser(const std::shared_ptr<Client> &client) noexcept {

    if (_isInviteOnly()) {
        return IRCCodes::INVITEONLYCHAN;
    }

    return _addUser(client);
}

IRCCodes Channel::removeUser(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_users.begin(), _users.end(), client);

    if (it != _users.end()) {
        _users.erase(client);
        _usersActive = _users.size();
        removeOperator(client);

        std::string partMessage = ":" + client->getNickname() + "!" +
                                  client->getUsername() + "@" +
                                  client->getIP() + " PART " + _channelName;
        for (const std::shared_ptr<Client> &user : _users) {
            user->appendMessageToQue(formatMessage(partMessage));
        }

        std::string userList = _allUsersInChannel();
        for (const std::shared_ptr<Client> &user : _users) {
            user->appendMessageToQue(
                formatMessage(":", _serverName, " 353 ", client->getNickname(),
                              " = ", _channelName, " :", userList));
            user->appendMessageToQue(
                formatMessage(":", _serverName, " 366 ", client->getNickname(),
                              " ", _channelName, " :End of /NAMES list"));
        }
        return IRCCodes::SUCCES;
    }

    return IRCCodes::USERNOTINCHANNEL;
}

void Channel::banUser(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_banned.begin(), _banned.end(), client);

    if (it != _banned.end()) {
        // check if user is in channel
        std::cout << "Add user to the ban list" << '\n';
    } else {
        std::cerr
            << "User already in banned from channel. send error code back??"
            << '\n';
    }
}

void Channel::unbanUser(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_banned.begin(), _banned.end(), client);

    if (it != _banned.end()) {
        std::cout << "Remove user from the ban list" << '\n';
    } else {
        std::cerr << "User was not in the ban list of the channel. send error "
                     "code back?? "
                  << '\n';
    }
}

void Channel::addOperator(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), client);

    if (it == _operators.end()) {
        if (_isBannedUser(client)) {
            std::cerr << "User: " << client
                      << " is banned from the channel: " << _channelName
                      << '\n';
            // send back erro code??
            return;
        }

        _operators.emplace(client);
        client->appendMessageToQue(formatMessage(
            ":", _serverName, " NOTICE ", client->getNickname(), " ",
            _channelName, " :Channel created. You are the operator"));
        std::cout << "User: " << client->getNickname()
                  << " is now a operator of channel: " << _channelName << '\n';
    } else {
        std::cerr << "User: " << client
                  << " is already a operator of channel: " << _channelName
                  << '\n';
        // send error code back??;
    }
}

void Channel::removeOperator(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), client);

    if (it != _operators.end()) {
        std::cout << "Remove user as operator" << " " << _usersActive << '\n';
        _operators.erase(client);
        if (!_users.empty() && _operators.empty()) {
            std::shared_ptr<Client> newOperator = *_users.begin();
            addOperator(newOperator);
        }
    } else {
        std::cerr << "User was never a operator. send error code back??"
                  << '\n';
    }
}

IRCCodes Channel::modeI(const std::string &state,
                        const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client) != true) {
        return IRCCodes::CHANOPRIVSNEEDED;
    }

    std::cout << "'" << state[0] << "'" << '\n';
    if (state[0] == '-') {
        _inviteOnly = false;
    } else {
        _inviteOnly = true;
    }

    return IRCCodes::SUCCES;
}

size_t Channel::usersActive() const noexcept {
    return _usersActive;
}

std::string Channel::channelName() const noexcept {
    return _channelName;
}

void Channel::sendMessage(const std::string &message,
                          const std::string &userID) noexcept {
    _broadcastMessage(message, "PRIVMSG", userID);
}

IRCCodes Channel::setTopic(const std::string &topic,
                           const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client)) {
        _topic = topic;
        _broadcastMessage(" :" + topic, "TOPIC", client->getFullID());
        return IRCCodes::SUCCES;
    }

    return IRCCodes::CHANOPRIVSNEEDED;
}

IRCCodes Channel::kickUser(const std::shared_ptr<Client> &user,
                           const std::shared_ptr<Client> &client) noexcept {
    if (_isOperator(client)) {
        if (user == client) {
            return IRCCodes::UNKNOWNCOMMAND;
        }
        _broadcastMessage(" " + user->getNickname() + " :Bye", "KICK ",
                          client->getFullID());
        removeUser(user);
    }

    return IRCCodes::CHANOPRIVSNEEDED;
}

IRCCodes Channel::inviteUser(const std::shared_ptr<Client> &user,
                             const std::shared_ptr<Client> &client) noexcept {

    if (_isOperator(client) != true) {
        return IRCCodes::CHANOPRIVSNEEDED;
    }

    return _addUser(user);
}

bool Channel::_isOperator(const std::shared_ptr<Client> &user) const noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), user);

    if (it != _operators.end()) {
        return true;
    }

    return false;
}

bool Channel::_isBannedUser(
    const std::shared_ptr<Client> &client) const noexcept {
    auto it = std::find(_banned.begin(), _banned.end(), client);

    if (it != _banned.end()) {
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

IRCCodes Channel::_addUser(const std::shared_ptr<Client> &client) noexcept {
    std::string nickname = client->getNickname();
    auto it = std::find(_users.begin(), _users.end(), client);

    if (it != _users.end()) {
        return IRCCodes::USERONCHANNEL;
    }

    if (_isBannedUser(client)) {
        return IRCCodes::BANNEDFROMCHAN;
    }

    if (_usersActive >= _userLimit) {
        return IRCCodes::CHANNELISFULL;
    }

    _users.emplace(client);
    _usersActive = _users.size();

    std::string userList = _allUsersInChannel();
    std::string joinMessage = ":" + client->getNickname() + "!" +
                              client->getUsername() + "@" + client->getIP() +
                              " JOIN " + _channelName;
    for (const std::shared_ptr<Client> &user : _users) {
        user->appendMessageToQue(formatMessage(joinMessage));
        user->appendMessageToQue(formatMessage(":", _serverName, " 332 ",
                                               nickname, " ", _channelName,
                                               " :", _topic));
        user->appendMessageToQue(formatMessage(":", _serverName, " 353 ",
                                               nickname, " = ", _channelName,
                                               " :", userList));
        user->appendMessageToQue(formatMessage(":", _serverName, " 366 ",
                                               nickname, " ", _channelName,
                                               " :End of /NAMES list"));
    }

    return IRCCodes::SUCCES;
}

std::string Channel::_allUsersInChannel() const noexcept {
    std::string userList;
    for (const std::shared_ptr<Client> &user : _users) {
        userList += (_isOperator(user) ? "@" : "") + user->getNickname() + " ";
    }

    return userList;
}
