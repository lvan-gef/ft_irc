/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Channel.cpp                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/10 21:16:09 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/10 21:16:09 by lvan-gef      ########   odam.nl         */
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
      _userLimit(USERLIMIT), _usersActive(1), _users{}, _banned{},
      _operators{} {
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
}

void Channel::init(const std::shared_ptr<Client> &client) {
    int clientFD = client->getFD();
    std::string nickname = client->getNickname();

    addUser(client);
    addOperator(client);

    sendMessage(clientFD, _serverName, "332 ", nickname, " ", _channelName,
                " :", _topic);

    std::string userList;
    for (const std::shared_ptr<Client> &user : _users) {
        userList += (isOperator(user) ? "@" : "") + user->getNickname() + " ";
    }
    sendMessage(clientFD, _serverName, "353 ", nickname, " = ", _channelName,
                " :", userList);
    sendMessage(clientFD, _serverName, "366 ", nickname, " ", _channelName,
                " :End of /NAMES list");

    sendMessage(clientFD, _serverName, "NOTICE ", nickname, " ", _channelName,
                " :Channel created. You are the operator");
}

void Channel::addUser(const std::shared_ptr<Client> &client) noexcept {
    std::string nickname = client->getNickname();
    auto it = std::find(_users.begin(), _users.end(), client);

    if (it != _users.end()) {
        sendMessage(client->getFD(), _serverName, "443 ", nickname, " ",
                    _channelName, " :is already on channel");
        return;
    }

    if (isBannedUser(client)) {
        sendMessage(client->getFD(), _serverName, "474 ", nickname, " ",
                    _channelName, " :Cannot join channel (+b)");
        return;
    }

    if (_usersActive >= _userLimit) {
        sendMessage(client->getFD(), _serverName, "471 ", nickname, " ",
                    _channelName, " :Cannot join channel (+l)");
        return;
    }

    _users.emplace(client);
    _usersActive = _users.size();

    std::string joinMessage = client->getNickname() + "!" +
                              client->getUsername() + "@" + client->getIP() +
                              " JOIN " + _channelName;
    for (const std::shared_ptr<Client> &user : _users) {
        sendMessage(user->getFD(), joinMessage);
    }

    std::cout << "User: " << nickname
              << " is added to channel: " << _channelName << '\n';
}

void Channel::removeUser(const std::shared_ptr<Client> &client) noexcept {
    auto it = std::find(_users.begin(), _users.end(), client);

    if (it != _users.end()) {
        std::cout << "remove user from the channel" << '\n';
        _usersActive = _users.size();
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
        std::cout << "User: " << client
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
        std::cout << "Remove user as operator" << '\n';
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
