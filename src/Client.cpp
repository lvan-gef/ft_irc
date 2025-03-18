/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/19 18:05:33 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/17 20:43:13 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <utility>
#include <vector>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"

Client::Client(int fd)
    : _fd(fd), _username(""), _nickname(""), _ip("0.0.0.0"),
      _partial_buffer(""), _messages{}, _offset(0), _event{}, _last_seen(0),
      _channels{} {
    _event.data.fd = _fd.get();
    _event.events = EPOLLIN | EPOLLOUT;
    _channels.reserve(EVENT_SIZE);
}

Client::Client(Client &&rhs) noexcept
    : _fd(std::move(rhs._fd)), _username(std::move(rhs._username)),
      _nickname(std::move(rhs._nickname)),
      _ip(std::move(rhs._ip)), _partial_buffer(std::move(rhs._partial_buffer)),
      _messages(std::move(rhs._messages)), _offset(rhs._offset),
      _event(rhs._event), _last_seen(rhs._last_seen),
      _channels(std::move(rhs._channels)) {
    rhs._fd = -1;
}

Client &Client::operator=(Client &&rhs) noexcept {
    if (this != &rhs) {
        _fd = std::move(rhs._fd);
        _username = std::move(rhs._username);
        _nickname = std::move(rhs._nickname);
        _partial_buffer = std::move(rhs._partial_buffer);
        _ip = std::move(rhs._ip);
        _event = rhs._event;
        _messages = std::move(rhs._messages);
        _last_seen = rhs._last_seen;
        _channels = std::move(rhs._channels);
        _offset = rhs._offset;
    }

    return *this;
}

Client::~Client() {
    std::cout << "Client destructor is called" << '\n';
    removeAllChannels();
}

int Client::getFD() const noexcept {
    return _fd.get();
}

epoll_event &Client::getEvent() noexcept {
    return _event;
}

bool Client::isRegistered() const noexcept {
    return _registered.all();
}

void Client::setUsername(const std::string &username) noexcept {
    setUsernameBit();
    _username = username;
}

void Client::setNickname(const std::string &nickname) noexcept {
    setNicknameBit();
    _nickname = nickname;
}

const std::string &Client::getUsername() const noexcept {
    return _username;
}

const std::string &Client::getNickname() const noexcept {
    return _nickname;
}

void Client::updatedLastSeen() noexcept {
    _last_seen = time(nullptr);
}

time_t Client::getLastSeen() const noexcept {
    return _last_seen;
}

void Client::setUsernameBit() noexcept {
    _registered.set(0);
}

void Client::setNicknameBit() noexcept {
    _registered.set(1);
}

void Client::setPasswordBit() noexcept {
    _registered.set(2);
}

bool Client::getUsernameBit() const noexcept {
    return _registered.test(0);
}

bool Client::getNicknameBit() const noexcept {
    return _registered.test(1);
}

bool Client::getPasswordBit() const noexcept {
    return _registered.test(2);
}

std::string Client::getFullID() const noexcept {
    return getNickname() + "!" + getUsername() + "@" + getIP();

}

void Client::setIP(const std::string &ip) noexcept {
    _ip = ip;
}
const std::string &Client::getIP() const noexcept {
    return _ip;
}

void Client::appendToBuffer(const std::string &data) noexcept {
    _partial_buffer += data;
}

std::string Client::getAndClearBuffer() {
    std::string tmp = _partial_buffer;
    _partial_buffer.clear();
    return tmp;
}

bool Client::hasCompleteMessage() const noexcept {
    return _partial_buffer.find("\r\n") != std::string::npos;
}

std::string Client::getMessage() {
    return _messages.front();
}

void Client::removeMessage() {
    _messages.pop();
    _offset = 0;
}

bool Client::haveMessagesToSend() {
    if (!_messages.empty()) {
        return true;
    }

    return false;
}

void Client::appendMessageToQue(const std::string &msg) noexcept {
    _messages.emplace(msg);
}

void Client::addChannel(const std::string &channelName) noexcept {
    auto it = std::find(_channels.begin(), _channels.end(), channelName);

    if (it != _channels.end()) {
        _channels.emplace_back(channelName);
    }
}
void Client::removeChannel(const std::string &channelName) noexcept {
    auto it = std::find(_channels.begin(), _channels.end(), channelName);

    if (it != _channels.end()) {
        _channels.erase(it);
    }
}

void Client::removeAllChannels() noexcept {
    std::vector<std::string> channels = allChannels();

    for (const std::string &channel : channels) {
        removeChannel(channel);
    }
}

std::vector<std::string> Client::allChannels() noexcept {
    return _channels;
}

void Client::setOffset(size_t offset) noexcept {
    _offset += offset;
}

size_t Client::getOffset() const noexcept {
    return _offset;
}
