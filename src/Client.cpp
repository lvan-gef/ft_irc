#include <algorithm>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <utility>
#include <vector>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"

Client::Client(int fd)
    : _fd(fd), _username(""), _nickname(""), _ip("0.0.0.0"), _realname(""),
      _partial_buffer(""), _messages{}, _event{}, _channels{} {
    _event.data.fd = _fd.get();
    _event.events = EPOLLIN | EPOLLOUT;
    _channels.reserve(static_cast<size_t>(Defaults::EVENT_SIZE));
}

Client::Client(Client &&rhs) noexcept
    : _epollNotifier(rhs._epollNotifier), _fd(std::move(rhs._fd)),
      _username(std::move(rhs._username)), _nickname(std::move(rhs._nickname)),
      _ip(std::move(rhs._ip)), _realname(std::move(rhs._realname)),
      _partial_buffer(std::move(rhs._partial_buffer)),
      _messages(std::move(rhs._messages)), _offset(rhs._offset),
      _event(rhs._event), _registered(rhs._registered),
      _disconnect(rhs._disconnect), _channels(std::move(rhs._channels)) {
    rhs._fd = -1;
}

Client &Client::operator=(Client &&rhs) noexcept {
    if (this != &rhs) {
        _epollNotifier = rhs._epollNotifier;
        _fd = std::move(rhs._fd);
        _username = std::move(rhs._username);
        _nickname = std::move(rhs._nickname);
        _partial_buffer = std::move(rhs._partial_buffer);
        _ip = std::move(rhs._ip);
        _event = rhs._event;
        _messages = std::move(rhs._messages);
        _offset = rhs._offset;
        _disconnect = rhs._disconnect;
        _channels = std::move(rhs._channels);
        _registered = rhs._registered;
        _realname = std::move(rhs._realname);
    }

    return *this;
}

Client::~Client() {
    removeAllChannels();
}

void Client::setEpollNotifier(EpollInterface *notifier) {
    _epollNotifier = notifier;
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

void Client::setRealname(const std::string &realname) noexcept {
    _realname = realname;
}

void Client::setNickname(const std::string &nickname) noexcept {
    setNicknameBit();
    _nickname = nickname;
}

const std::string &Client::getUsername() const noexcept {
    return _username;
}

const std::string &Client::getRealname() const noexcept {
    return _realname;
}

const std::string &Client::getNickname() const noexcept {
    return _nickname;
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

std::string Client::getAndClearBuffer() noexcept {
    size_t index = _partial_buffer.find("\r\n");
    if (index == std::string::npos) {
        std::cerr << "Find \\r\\n failed when getting the buffer" << '\n';
        return "";
    }

    std::string tmp;
    if (index == _partial_buffer.length()) {
        tmp = _partial_buffer;
        _partial_buffer.clear();
    } else {
        tmp = _partial_buffer.substr(0, index);
        _partial_buffer.erase(0, index + 2);
    }

    return tmp;
}

bool Client::hasCompleteMessage() const noexcept {
    return _partial_buffer.find("\r\n") != std::string::npos;
}

std::string Client::getMessage() noexcept {
    return _messages.front();
}

void Client::removeMessage() noexcept {
    _messages.pop();
    _offset = 0;
}

bool Client::haveMessagesToSend() noexcept {
    if (_messages.empty() != true) {
        return true;
    }

    return false;
}

void Client::appendMessageToQue(const std::string &msg) noexcept {
    _messages.emplace(msg);
    if (_epollNotifier) {
        _epollNotifier->notifyEpollUpdate(_fd.get());
    }
}

void Client::setDisconnect() noexcept {
    _disconnect = true;
}

bool Client::isDisconnect() const noexcept {
    return _disconnect;
}

void Client::addChannel(const std::string &channelName) noexcept {
    auto it = std::find(_channels.begin(), _channels.end(), channelName);

    if (it == _channels.end()) {
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

std::vector<std::string> Client::allChannels() const noexcept {
    return _channels;
}

void Client::setOffset(size_t offset) noexcept {
    _offset += offset;
}

size_t Client::getOffset() const noexcept {
    return _offset;
}
