#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "./Channel.hpp"
#include "./Client.hpp"
#include "./EpollInterface.hpp"
#include "./FileDescriptor.hpp"
#include "./Token.hpp"

struct ApiRequest {
    int fd;
    std::shared_ptr<Client> client;
    std::string buffer;
    std::string request;
    enum State { CONNECTING, SENDING, READING } state;
};

class Server final : public EpollInterface {
  public:
    explicit Server(const std::string &port, std::string &password);

    Server(const Server &rhs) = delete;
    Server &operator=(const Server &rhs) = delete;

    Server(Server &&rhs) noexcept;
    Server &operator=(Server &&rhs) noexcept;

    ~Server() override;

  public:
    class ServerException final : public std::exception {
      public:
        const char *what() const noexcept override;
    };

  public:
    void notifyEpollUpdate(int fd) override;

  public:
    bool init() noexcept;
    bool run() noexcept;

  public:
    std::string getChannelsAndUsers() noexcept;
    int getEpollFD() const noexcept;
    void addApiRequest(const ApiRequest &api) noexcept;

  private:
    bool _init() noexcept;
    void _run();
    void _shutdown() noexcept;
    static int _setNonBlocking(int fd) noexcept;

  private:
    void _newConnection() noexcept;
    void _clientAccepted(const std::shared_ptr<Client> &client) noexcept;
    void _clientRecv(int fd) noexcept;
    void _clientSend(int fd) noexcept;
    void _removeClient(const std::shared_ptr<Client> &client) noexcept;
    Channel *isChannel(const std::string &channelName) noexcept;

  private:
    void _processMessage(const std::shared_ptr<Client> &client) noexcept;
    void _handleCommand(const IRCMessage &token,
                        const std::shared_ptr<Client> &client) noexcept;

  private:
    void _handleNickname(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept;
    void _handleUsername(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept;
    void _handlePassword(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) const noexcept;
    void _handlePriv(const IRCMessage &token,
                     const std::shared_ptr<Client> &client) noexcept;
    void _handleJoin(const IRCMessage &token,
                     const std::shared_ptr<Client> &client) noexcept;
    void _handleTopic(const IRCMessage &token,
                      const std::shared_ptr<Client> &client) noexcept;
    void _handlePart(const IRCMessage &token,
                     const std::shared_ptr<Client> &client) noexcept;
    static void _handlePing(const IRCMessage &token,
                            const std::shared_ptr<Client> &client) noexcept;
    void _handleKick(const IRCMessage &token,
                     const std::shared_ptr<Client> &client) noexcept;
    void _handleInvite(const IRCMessage &token,
                       const std::shared_ptr<Client> &client) noexcept;
    void _handleMode(const IRCMessage &token,
                     const std::shared_ptr<Client> &client) noexcept;
    void _handleUserhost(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) const noexcept;
    static void _handleUnkown(const IRCMessage &token,
                              const std::shared_ptr<Client> &client) noexcept;
    void _handleWhois(const IRCMessage &token,
                      const std::shared_ptr<Client> &client) const noexcept;

  private:
    std::uint16_t _port;
    std::string _password;
    std::string _serverStared;

  private:
    FileDescriptor _server_fd;
    FileDescriptor _epoll_fd;
    std::size_t _connections{0};

  private:
    std::unordered_map<int, std::shared_ptr<Client>> _fd_to_client;
    std::unordered_map<std::string, std::shared_ptr<Client>> _nick_to_client;
    std::unordered_map<std::string, Channel> _channels;
    std::unordered_map<int, ApiRequest> _api_requests;
};

#endif // !SERVER_HPP
