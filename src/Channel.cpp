/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:09 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/11 20:53:15 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "../include/Channel.hpp"
#include "../include/Client.hpp"
#include "../include/Enums.hpp"

Channel::Channel(const std::string &serverName, const std::string &channelName,
                 const std::string &channelTopic,
                 const std::shared_ptr<Client> &client)
    : _serverName(serverName), _channelName(channelName), _topic(channelTopic),
      _userLimit(USERLIMIT), _usersActive(1), _users{}, _banned{},
      _operators{}, _inviteOnly(false) {
    init(client);
}

Channel::Channel(Channel &&rhs) noexcept
    : _serverName(std::move(rhs._serverName)),
      _channelName(std::move(rhs._channelName)), _topic(std::move(rhs._topic)),
      _userLimit(rhs._userLimit), _usersActive(rhs._usersActive),
      _users(std::move(rhs._users)), _banned(std::move(rhs._banned)),
      _operators(std::move(rhs._operators)) {
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

void Channel::addUser(const std::shared_ptr<Client> &client) noexcept {
    std::string nickname = client->getNickname();
    auto it = std::find(_users.begin(), _users.end(), client);

    if (it != _users.end()) {
        client->appendMessageToQue(_serverName, "443 ", nickname, " ",
                                   _channelName, " :is already on channel");
        return;
    }

    if (isBannedUser(client)) {
        client->appendMessageToQue(_serverName, "474 ", nickname, " ",
                                   _channelName, " :Cannot join channel (+b)");
        return;
    }

    if (_usersActive >= _userLimit) {
        client->appendMessageToQue(_serverName, "471 ", nickname, " ",
                                   _channelName, " :Cannot join channel (+l)");
        return;
    }

    _users.emplace(client);
    _usersActive = _users.size();

    std::string userList = allUsersInChannel();
    std::string joinMessage = client->getNickname() + "!" +
                              client->getUsername() + "@" + client->getIP() +
                              " JOIN " + _channelName;
    for (const std::shared_ptr<Client> &user : _users) {
        user->appendMessageToQue(_serverName, "332 ", nickname, " ",
                                 _channelName, " :", _topic);
        user->appendMessageToQue(joinMessage);
        user->appendMessageToQue(_serverName, "353 ", nickname, " = ",
                                 _channelName, " :", userList);
        user->appendMessageToQue(_serverName, "366 ", nickname, " ",
                                 _channelName, " :End of /NAMES list");
    }

    std::cout << "User: " << nickname
              << " is added to channel: " << _channelName << '\n';
}

void Channel::removeUser(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_users.begin(), _users.end(), client);

    if (it != _users.end()) {
        std::cout << "remove user from the channel" << '\n';
        _users.erase(client);
        _usersActive = _users.size();
        removeOperator(client);

        std::string userList = allUsersInChannel();
        for (const std::shared_ptr<Client> &user : _users) {
            user->appendMessageToQue(_serverName, "353 ", client->getNickname(),
                                     " = ", _channelName, " :", userList);
            user->appendMessageToQue(_serverName, "366 ", client->getNickname(),
                                     " ", _channelName, " :End of /NAMES list");
        }
    } else {
        std::cerr << "User not in channel. send error code back??" << '\n';
    }
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

bool Channel::isBannedUser(
    const std::shared_ptr<Client> &client) const noexcept {
    auto it = std::find(_banned.begin(), _banned.end(), client);

    if (it != _banned.end()) {
        return true;
    }

    return false;
}

void Channel::addOperator(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), client);

    if (it == _operators.end()) {
        if (isBannedUser(client)) {
            std::cerr << "User: " << client
                      << " is banned from the channel: " << _channelName
                      << '\n';
            // send back erro code??
            return;
        }

        _operators.emplace(client);
        client->appendMessageToQue(_serverName, "NOTICE ",
                                   client->getNickname(), " ", _channelName,
                                   " :Channel created. You are the operator");
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

bool Channel::isOperator(const std::shared_ptr<Client> &client) const noexcept {
    auto it = std::find(_operators.begin(), _operators.end(), client);

    if (it != _operators.end()) {
        return true;
    }

    return false;
}

size_t Channel::usersActive() const noexcept {
    return _usersActive;
}

std::string Channel::channelName() const noexcept {
    return _channelName;
}

std::string Channel::allUsersInChannel() const noexcept {
    std::string userList;
    for (const std::shared_ptr<Client> &user : _users) {
        userList += (isOperator(user) ? "@" : "") + user->getNickname() + " ";
    }

    return userList;
}

void Channel::broadcastMessage(const std::string &message) const noexcept {
    for (const std::shared_ptr<Client> &client : _users) {
        client->appendMessageToQue(_serverName, message);
    }
}

void Channel::setTopic(const std::string &topic) noexcept {
    _topic = topic;
}
