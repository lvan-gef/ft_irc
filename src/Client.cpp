#include <ctime>

#include <unistd.h>

#include "../include/Client.hpp"

Client::Client(int fd) : _fd(fd), _event(), _last_seen(0), _registered(false) {
    _event.data.fd = fd;
    _event.events = EPOLLIN | EPOLLET;
}

Client::Client(const Client &rhs)
    : _fd(rhs._fd), _event(rhs._event), _last_seen(rhs._last_seen),
      _registered(rhs._registered) {
}

Client &Client::operator=(const Client &rhs) {
    if (this != &rhs) {
        _fd = rhs._fd;
        _event = rhs._event;
        _last_seen = rhs._last_seen;
        _registered = rhs._registered;
    }

    return *this;
}

Client::Client(Client &&rhs) noexcept
    : _fd(rhs._fd), _event(rhs._event), _last_seen(rhs._last_seen),
      _registered(rhs._registered) {
}

Client &Client::operator=(Client &&rhs) noexcept {
    if (this != &rhs) {
        _fd = rhs._fd;
        _event = rhs._event;
        _last_seen = rhs._last_seen;
        _registered = rhs._registered;
    }

    return *this;
}

Client::~Client() {
    if (_fd >= 0) {
        close(_fd);
    }
}

int Client::getFD() const noexcept {
    return _fd;
}

const std::string &Client::getUsername() const noexcept {
    return _username;
}

const std::string &Client::getNickname() const noexcept {
    return _nickname;
}

epoll_event &Client::getEvent() noexcept {
    return _event;
}

bool Client::isRegistered() const noexcept {
    return _registered;
}

time_t Client::getLastSeen() const noexcept {
    return _last_seen;
}

void Client::setUsername(const std::string &username) noexcept {
    _username = username;
}

void Client::setNickname(const std::string &nickname) noexcept {
    _nickname = nickname;
}

void Client::setRegistered(bool registered) noexcept {
    _registered = registered;
}

void Client::updatedLastSeen() noexcept {
    _last_seen = time(nullptr);
}

void Client::appendToBuffer(const std::string &data) noexcept {
    _partial_buffer += data;
}

std::string Client::getAndClearBuffer() noexcept {
    std::string tmp = _partial_buffer;
    _partial_buffer.clear();
    return tmp;
}

bool Client::hasCompleteMessage() const noexcept {
    return _partial_buffer.find("\r\n") != std::string::npos;
}
