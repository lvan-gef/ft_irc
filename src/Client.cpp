/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/19 18:05:33 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/11 16:59:19 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <utility>

#include "../include/Client.hpp"

Client::Client(int fd)
    : _fd(fd), _username(""), _nickname(""), _partial_buffer(""),
      _ip("0.0.0.0"), _event{}, _messages{}, _last_seen(0) {
    _event.data.fd = _fd.get();
    _event.events = EPOLLIN | EPOLLOUT;
}

Client::Client(Client &&rhs) noexcept
    : _fd(std::move(rhs._fd)), _username(std::move(rhs._username)),
      _nickname(std::move(rhs._nickname)),
      _partial_buffer(std::move(rhs._partial_buffer)), _ip(std::move(rhs._ip)),
      _event(rhs._event), _messages(std::move(rhs._messages)), _last_seen(rhs._last_seen) {
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
    }

    return *this;
}

Client::~Client() {
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
    std::string msg = _messages.front();
    _messages.pop();

    return msg;
}

bool Client::haveMessagesToSend() {
    if (_messages.size() > 0) {
        return true;
    }

    return false;
}
