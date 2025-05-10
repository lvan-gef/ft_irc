#include <algorithm>
#include <cctype>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/Channel.hpp"
#include "../include/Chatbot.hpp"
#include "../include/Client.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/Utils.hpp"

namespace {
bool g_running{true};
void signalHandler(const int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        g_running = false;
    }
}
} // namespace

Server::Server(const std::string &port, std::string &password)
    : _port(toUint16(port)), _password(std::move(password)), _serverStared(""),
      _server_fd(-1), _epoll_fd(-1) {
    if (errno != 0) {
        throw std::invalid_argument("Invalid port");
    }

    if (_port < Defaults::LOWEST_PORT || _port > Defaults::HIGHEST_PORT) {
        throw std::range_error("Port is out of range");
    }

    if (_password.length() == 0) {
        throw std::invalid_argument("password can not be empty");
    }

    const auto now = std::chrono::system_clock::now();
    const std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm utc_time = {};

    std::memset(&utc_time, 0, sizeof(std::tm));
    if (gmtime_r(&now_time, &utc_time) == nullptr) {
        throw std::runtime_error("Failed to convert time to UTC");
    }

    char buffer[80];
    if (strftime(buffer, sizeof(buffer), "%a %b %d %Y at %H:%M:%S UTC",
                 &utc_time) == 0) {
        throw std::runtime_error("Time formatting failed");
    }

    _serverStared = buffer;
}

Server::Server(Server &&rhs) noexcept
    : _port(rhs._port), _password(std::move(rhs._password)),
      _serverStared(std::move(rhs._serverStared)),
      _server_fd(std::move(rhs._server_fd)),
      _epoll_fd(std::move(rhs._epoll_fd)), _connections(rhs._connections),
      _fd_to_client(std::move(rhs._fd_to_client)),
      _nick_to_client(std::move(rhs._nick_to_client)),
      _channels(std::move(rhs._channels)) {
}

Server &Server::operator=(Server &&rhs) noexcept {
    if (this != &rhs) {
        _port = rhs._port;
        _password = std::move(rhs._password);
        _serverStared = std::move(rhs._serverStared);
        _server_fd = std::move(rhs._server_fd);
        _epoll_fd = std::move(rhs._epoll_fd);
        _connections = rhs._connections;
        _fd_to_client = std::move(rhs._fd_to_client);
        _nick_to_client = std::move(rhs._nick_to_client);
        _channels = std::move(rhs._channels);
    }

    return *this;
}

Server::~Server() {
    _shutdown();
}

void Server::notifyEpollUpdate(const int fd) {
    const auto it = _fd_to_client.find(fd);
    if (it == _fd_to_client.end()) {
        return;
    }

    epoll_event ev = it->second->getEvent();
    ev.events = EPOLLIN | EPOLLOUT;
    if (0 > epoll_ctl(_epoll_fd.get(), EPOLL_CTL_MOD, fd, &ev)) {
        std::cerr << "Failed to update epoll: " << strerror(errno) << '\n';
    }
}

bool Server::init() noexcept {
    if (_server_fd.get() >= 0 || _epoll_fd.get() >= 0) {
        std::cerr << "Server already initialized" << '\n';
        return false;
    }

    return _init();
}

bool Server::run() noexcept {
    if (0 > _server_fd.get() || 0 > _epoll_fd.get()) {
        std::cerr << "Server is not initialized. Call init() first then run()"
                  << '\n';
        return false;
    }

    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        std::cerr << "Failed to set up SGINT handler" << '\n';
        return false;
    }
    if (sigaction(SIGTERM, &sa, nullptr) == -1) {
        std::cerr << "Failed to set up SIGTERM handler" << '\n';
        return false;
    }

    if (sigaction(SIGPIPE, &sa, nullptr) == -1) {
        std::cerr << "Failed to set up SIGTERM handler" << '\n';
        return false;
    }

    try {
        _run();
    } catch (const ServerException &e) {
        std::cerr << e.what() << '\n';
        return false;
    }

    return true;
}

const char *Server::ServerException::what() const noexcept {
    return "Internal server error";
}

std::string Server::getChannelsAndUsers() noexcept {
    std::stringstream ss;

    ss << "channels and users:\n";

    std::vector<std::reference_wrapper<Channel>> sortedChannels;
    sortedChannels.reserve(_channels.size());
    for (auto &pair : _channels) {
        sortedChannels.push_back(std::ref(pair.second));
    }

    std::sort(sortedChannels.begin(), sortedChannels.end(),
              [](const Channel &a, const Channel &b) {
                  return a.getName() < b.getName();
              });

    for (const auto &pair : sortedChannels) {
        const std::string &channelName = pair.get().getName();
        const std::string userListStr = pair.get().getUserList();
        if (userListStr.empty()) {
            continue;
        }
        ss << channelName << ": ";
        const std::vector<std::string> users = split(userListStr, " ");
        std::vector<std::string> sortedUsers = users;
        std::sort(sortedUsers.begin(), sortedUsers.end());
        sortedUsers.erase(
            std::remove_if(sortedUsers.begin(), sortedUsers.end(),
                           [](const std::string &s) { return s.empty(); }),
            sortedUsers.end());

        for (std::size_t i = 0; i < sortedUsers.size(); ++i) {
            ss << sortedUsers[i];
            if (i == sortedUsers.size() - 1)
                ss << ".";
            else
                ss << ", ";
        }
        ss << "\n";
    }

    return ss.str();
}

int Server::getEpollFD() const noexcept {
    return _epoll_fd.get();
}

void Server::addApiRequest(const ApiRequest &api) noexcept {
    _api_requests[api.fd] = api;
}

bool Server::_init() noexcept {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > _server_fd.get()) {
        std::cerr << "Failed to create a socket: " << strerror(errno) << '\n';
        return false;
    }

    constexpr int opt = 1;
    if (0 > setsockopt(_server_fd.get(), SOL_SOCKET, SO_REUSEADDR, &opt,
                       sizeof(opt))) {
        std::cerr << "setsockopt failed: " << strerror(errno) << '\n';
        return false;
    }

    if (0 > _setNonBlocking(_server_fd.get())) {
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(_port);
    if (0 > bind(_server_fd.get(), (sockaddr *)&address, sizeof(address))) {
        std::cerr << "Bind failed: " << strerror(errno) << '\n';
        return false;
    }

    if (0 > listen(_server_fd.get(), SOMAXCONN)) {
        std::cerr << "Listen failed: " << strerror(errno) << '\n';
        return false;
    }

    _epoll_fd = epoll_create1(0);
    if (0 > _epoll_fd.get()) {
        std::cerr << "Epoll create failed: " << strerror(errno) << '\n';
        return false;
    }

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = _server_fd.get();
    if (0 > epoll_ctl(_epoll_fd.get(), EPOLL_CTL_ADD, _server_fd.get(), &ev)) {
        std::cerr << "Epoll add failed: " << strerror(errno) << '\n';
        return false;
    }

    std::cout << "Server is running on: " << _port << ". Press Ctrl+C to stop."
              << '\n';
    return true;
}

void Server::_run() {
    std::vector<epoll_event> events(static_cast<int>(Defaults::EVENT_SIZE));

    while (g_running) {
        const int nfds = epoll_wait(
            _epoll_fd.get(), static_cast<epoll_event *>(events.data()),
            (int)events.size(), static_cast<int>(Defaults::INTERVAL));

        if (0 > nfds) {
            if (errno == EINTR) {
                continue;
            }

            std::cerr << "Epoll wait failed: " << strerror(errno) << '\n';
            throw ServerException();
        }

        for (int index = 0; index < nfds; ++index) {
            const auto &event = events[static_cast<std::size_t>(index)];

            if (event.data.fd == _server_fd.get()) {
                _newConnection();
            } else if (event.events & EPOLLIN) {
                auto api_it = _api_requests.find(event.data.fd);
                if (api_it != _api_requests.end()) {
                    ApiRequest &current_api_request = api_it->second;
                    handleRecvApi(current_api_request);

                    if (current_api_request.fd == -1) {
                        _api_requests.erase(api_it);
                    } else {
                        std::cout << "Server::_run: API request for fd="
                                  << event.data.fd << " waiting for more data."
                                  << '\n';
                    }
                } else {
                    _clientRecv(event.data.fd);
                }
            } else if (event.events & EPOLLOUT) {
                auto api_it = _api_requests.find(event.data.fd);
                if (api_it != _api_requests.end()) {
                    ApiRequest &current_api_request = api_it->second;
                    if (!handleSendApi(current_api_request, event,
                                       getEpollFD())) {
                        if (current_api_request.fd == -1) {
                            _api_requests.erase(api_it);
                        }
                    }
                } else {
                    _clientSend(event.data.fd);
                }
            } else if (event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                auto api_it = _api_requests.find(event.data.fd);
                if (api_it != _api_requests.end()) {
                    std::cerr << "Server::_run: EPOLLERR/HUP on API socket fd="
                              << event.data.fd << ". Removing request." << '\n';
                    if (api_it->second.fd != -1) {
                        close(api_it->second.fd);
                        api_it->second.fd = -1;
                    }
                    _api_requests.erase(api_it);
                } else {
                    auto it = _fd_to_client.find(event.data.fd);
                    if (it != _fd_to_client.end()) {
                        std::cerr
                            << "Server::_run: EPOLLERR/HUP on client socket fd="
                            << event.data.fd << ". Removing client." << '\n';
                        _removeClient(it->second);
                    } else {
                        std::cerr << "Server::_run: EPOLLERR/HUP on unknown fd="
                                  << event.data.fd << '\n';
                        epoll_ctl(_epoll_fd.get(), EPOLL_CTL_DEL, event.data.fd,
                                  nullptr);
                    }
                }
            } else {
                std::cout << "Unknown epoll event " << event.events
                          << " from fd: " << event.data.fd << '\n';
            }
        }
    }
}

void Server::_shutdown() noexcept {
    std::cout << '\n' << "Shutting down server..." << '\n';

    std::vector<std::shared_ptr<Client>> clients_to_remove;
    clients_to_remove.reserve(_fd_to_client.size());
    for (const auto &pair : _fd_to_client) {
        clients_to_remove.push_back(pair.second);
    }

    for (const auto &client : clients_to_remove) {
        _removeClient(client);
    }

    _fd_to_client.clear();
    _nick_to_client.clear();
    _channels.clear();
    _api_requests.clear();
}

int Server::_setNonBlocking(const int fd) noexcept {
    const int returnCode = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (0 > returnCode) {
        std::cerr << "Failed to set to non-blocking mode: " << strerror(errno)
                  << '\n';
    }

    return returnCode;
}

void Server::_newConnection() noexcept {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);

    int clientFD =
        accept(_server_fd.get(), reinterpret_cast<sockaddr *>(&clientAddr),
               &clientLen);

    if (0 > clientFD) {
        std::cerr << "Accept failed: " << strerror(errno) << '\n';
        return;
    }

    if (0 > _setNonBlocking(clientFD)) {
        close(clientFD);
        return;
    }

    const std::shared_ptr<Client> client = std::make_shared<Client>(clientFD);
    client->setEpollNotifier(this);
    if (0 > epoll_ctl(_epoll_fd.get(), EPOLL_CTL_ADD, clientFD,
                      &client->getEvent())) {
        std::cerr << "Failed to add client to epoll" << '\n';
        return;
    }

    client->setIP(inet_ntoa(clientAddr.sin_addr));
    _fd_to_client[clientFD] = client;
    _connections = _fd_to_client.size();

    std::cout << "New client connected on fd " << clientFD << '\n';
}

void Server::_clientAccepted(const std::shared_ptr<Client> &client) noexcept {
    if (client->isRegistered() != true) {
        return;
    }

    const int clientFD = client->getFD();
    const std::string nick = client->getNickname();
    handleMsg(IRCCode::WELCOME, client, "", "");
    handleMsg(IRCCode::YOURHOST, client, "", "");
    handleMsg(IRCCode::CREATED, client, "", _serverStared);
    handleMsg(IRCCode::MYINFO, client, "", "o i,t,k,o,l k,l,o");
    handleMsg(IRCCode::ISUPPORT, client, "", "");
    handleMsg(IRCCode::MOTDSTART, client, "", "");
    handleMsg(IRCCode::MOTD, client, "",
              "- This server is for educational purposes!");
    handleMsg(IRCCode::MOTD, client, "", "- Have fun chatting!");
    handleMsg(IRCCode::ENDOFMOTD, client, "", "");

    _nick_to_client[nick] = client;
    std::cerr << "Client on fd: " << clientFD << " is accepted" << '\n';
}

void Server::_clientRecv(const int fd) noexcept {
    const std::shared_ptr<Client> client = _fd_to_client[fd];
    if (!client) {
        return;
    }

    char buffer[static_cast<int>(Defaults::READ_SIZE) + 1] = {};
    const ssize_t bytes_read =
        recv(fd, buffer, static_cast<int>(Defaults::READ_SIZE), MSG_DONTWAIT);
    if (0 > bytes_read) {
        if (errno == EAGAIN) {
            return;
        }

        _removeClient(client);
        std::cerr << "Error while recv: " << strerror(errno) << '\n';
        return;
    }

    if (bytes_read == 0) {
        _removeClient(client);
        return;
    }

    std::cout << "recv from fd: " << client->getFD() << ": " << buffer << '\n';
    client->appendToBuffer(std::string(buffer, (std::size_t)bytes_read));
    _processMessage(client);
}

void Server::_clientSend(const int fd) noexcept {
    const std::shared_ptr<Client> client = _fd_to_client[fd];

    while (client->haveMessagesToSend()) {
        const std::string msg = client->getMessage();

        const std::size_t offset = client->getOffset();
        std::cout << "send to fd: " << client->getFD() << ": " << msg.c_str()
                  << '\n';
        const ssize_t bytes =
            send(client->getFD(), msg.c_str() + offset, msg.length() - offset,
                 MSG_DONTWAIT | MSG_NOSIGNAL);

        if (0 > bytes) {
            if (errno == EAGAIN) {
                break;
            }

            std::cerr << "Error while sending: " << strerror(errno) << '\n';
            _removeClient(client);
            break;
        }

        client->setOffset(static_cast<std::size_t>(bytes));
        if (client->getOffset() >= msg.length()) {
            client->removeMessage();
        } else {
            break;
        }
    }

    epoll_event ev = client->getEvent();
    if (client->haveMessagesToSend() != true) {
        ev.events = EPOLLIN;
        if (epoll_ctl(_epoll_fd.get(), EPOLL_CTL_MOD, client->getFD(), &ev) ==
            -1) {
            std::cerr << "Failed to update epoll event: " << strerror(errno)
                      << '\n';
        }

        if (client->isDisconnect()) {
            _removeClient(client);
        }
    }
}

void Server::_removeClient(const std::shared_ptr<Client> &client) noexcept {
    if (!client) {
        return;
    }

    const int fd = client->getFD();
    const std::string &nickname = client->getNickname();

    try {
        const auto fd_it = _fd_to_client.find(fd);
        const auto nick_it = _nick_to_client.find(nickname);

        if (fd_it != _fd_to_client.end() && fd_it->second == client) {
            _fd_to_client.erase(fd_it);
            epoll_ctl(_epoll_fd.get(), EPOLL_CTL_DEL, fd, nullptr);
            _connections = _fd_to_client.size();
        }

        if (nick_it != _nick_to_client.end() && nick_it->second == client) {
            _nick_to_client.erase(nick_it);
        }

        const std::vector<std::string> &channels = client->allChannels();
        for (const std::string &channel : channels) {
            auto it = _channels.find(channel);
            if (it != _channels.end()) {
                it->second.removeUser(client, "");
            }
        }

        std::cout << "Client FD: " << fd << " disconnected" << '\n';
    } catch (const std::exception &e) {
        std::cerr << "Error while removing client - FD: " << fd
                  << " Nickname: '" << nickname << "' - " << e.what() << '\n';
    }
}

Channel *Server::isChannel(const std::string &channelName) noexcept {
    std::string channelUpper = channelName;
    std::transform(channelUpper.begin(), channelUpper.end(),
                   channelUpper.begin(), ::toupper);

    for (auto &it : _channels) {
        std::string uppercaseIt = it.first;
        std::transform(uppercaseIt.begin(), uppercaseIt.end(),
                       uppercaseIt.begin(), ::toupper);
        if (uppercaseIt == channelUpper) {
            return &it.second;
        }
    }

    return nullptr;
}

void Server::_processMessage(const std::shared_ptr<Client> &client) noexcept {
    while (client->hasCompleteMessage()) {

        const std::string msg = client->getAndClearBuffer();

        if (msg == "") {
            return;
        }

        if (msg.length() > getDefaultValue(Defaults::MAXMSGLEN)) {
            handleMsg(IRCCode::INPUTTOOLONG, client, "", "");
            continue;
        }

        std::cout << "message from fd: " << client->getFD() << ": " << msg << '\n';
        const std::vector<IRCMessage> clientsToken = parseIRCMessage(msg);
        for (const IRCMessage &token : clientsToken) {
            if (!token.succes) {
                try {
                    handleMsg(token.err.get_value(), client, token.errMsg,
                              "Unknow command");
                } catch (std::runtime_error &e) {
                    std::cerr << "Failed to get value from err: " << e.what()
                              << '\n';
                    return;
                }
            } else {
                _handleCommand(token, client);
            }
        }
    }
}
