/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/19 17:48:55 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/27 17:15:45 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <cstdint>
#include <memory>
#include <unordered_map>

#include <sys/socket.h>

#include "./Channel.hpp"
#include "./Client.hpp"
#include "./FileDescriptors.hpp"
#include "./Token.hpp"
#include "./EpollNotifier.hpp"

class Server : public EpollNotifier {
  public:
    explicit Server(const std::string &port, std::string &password);

    Server(const Server &rhs) = delete;
    Server &operator=(const Server &rhs) = delete;

    Server(Server &&rhs) noexcept;
    Server &operator=(Server &&rhs) noexcept;

    ~Server() override;

  public:
    class ServerException : public std::exception {
        public:
            const char *what() const noexcept override;
    };

  public:
    void notifyEpollUpdate(int fd) override;

  public:
    bool init() noexcept;
    bool run() noexcept;

  private:
    bool _init() noexcept;
    void _run();
    void _shutdown() noexcept;
    int _setNonBlocking(int fd) noexcept;

  private:
    void _newConnection() noexcept;
    void _clientAccepted(const std::shared_ptr<Client> &client) noexcept;
    void _clientRecv(int fd) noexcept;
    void _clientSend(int fd) noexcept;
    void _removeClient(const std::shared_ptr<Client> &client) noexcept;

  private:
    void _processMessage(const std::shared_ptr<Client> &client) noexcept;
    void _handleCommand(const IRCMessage &token,
                        const std::shared_ptr<Client> &client);

  private:
    void _handleNickname(const IRCMessage &token,
                         const std::shared_ptr<Client> &client);
    void _handleUsername(const IRCMessage &token,
                         const std::shared_ptr<Client> &client);
    void _handlePassword(const IRCMessage &token,
                         const std::shared_ptr<Client> &client);
    void _handlePriv(const IRCMessage &token,
                     const std::shared_ptr<Client> &client);
    void _handleJoin(const IRCMessage &token,
                     const std::shared_ptr<Client> &client);
    void _handleTopic(const IRCMessage &token,
                      const std::shared_ptr<Client> &client);
    void _handlePart(const IRCMessage &token,
                     const std::shared_ptr<Client> &client);
    void _handleQuit(const IRCMessage &token,
                     const std::shared_ptr<Client> &client);
    void _handlePing(const IRCMessage &token,
                     const std::shared_ptr<Client> &client);
    void _handleKick(const IRCMessage &token,
                     const std::shared_ptr<Client> &client);
    void _handleInvite(const IRCMessage &token,
                       const std::shared_ptr<Client> &client);
    void _handleMode(const IRCMessage &token,
                     const std::shared_ptr<Client> &client);
    void _handleUserhost(const IRCMessage &token,
                         const std::shared_ptr<Client> &client);

  private:
    std::uint16_t _port;
    std::string _password;
    std::string _serverStared;

  private:
    FileDescriptors _server_fd;
    FileDescriptors _epoll_fd;
    size_t _connections;

  private:
    std::unordered_map<int, std::shared_ptr<Client>> _fd_to_client;
    std::unordered_map<std::string, std::shared_ptr<Client>> _nick_to_client;
    std::unordered_map<std::string, Channel> _channels;
};

#endif // !SERVER_HPP
