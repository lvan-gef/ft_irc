/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerMessageHelper.cpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/11 21:16:33 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <memory>

#include "../include/Channel.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"

void Server::_handleMessage(const IRCMessage &token,
                            const std::shared_ptr<Client> &client) {

    std::string clientNick = client->getNickname();

    switch (token.type) {
        case IRCCommand::NICK:
            client->setNickname(token.params[0]);
            _clientAccepted(client);
            break;
        case IRCCommand::USER:
            client->setUsername(token.params[0]);
            _clientAccepted(client);
            break;
        case IRCCommand::PASS:
            client->setPasswordBit();
            _clientAccepted(client);
            break;
        case IRCCommand::PRIVMSG:
            if (token.params[0][0] == '#') {
                auto it = _channels.find(token.params[0]);
                if (it != _channels.end()) {
                    it->second.broadcastMessage(token.params[1]);
                }
            } else {
                auto it = _nick_to_client.find(token.params[0]);
                if (it != _nick_to_client.end()) {
                    it->second->appendMessageToQue(clientNick, "PRIVMSG ",
                                                   clientNick, " :",
                                                   token.params[1]);
                } else {
                    std::cerr << "no client with that name send errror back";
                }
            }
            break;
        case IRCCommand::CREATE:
            if (_channels.find(token.params[0]) == _channels.end()) {
                std::string topic =
                    token.params.size() > 1 ? token.params[1] : "Default";
                _channels.emplace(
                    token.params[0],
                    Channel(_serverName, token.params[0], topic, client));
            } else {
                std::cerr << "Impl channel does exists" << '\n';
            }
            break;
        case IRCCommand::JOIN: {
            auto channel = _channels.find(token.params[0]);
            if (channel == _channels.end()) {
                std::string topic =
                    token.params.size() > 1 ? token.params[1] : "Default";
                _channels.emplace(
                    token.params[0],
                    Channel(_serverName, token.params[0], topic, client));
            } else {
                if (channel->second.inviteOnly()) {
                    std::cerr << "channel invite only. send back error" << '\n';
                }
                channel->second.addUser(client);
            }
            break;
        }
        case IRCCommand::TOPIC: {
            auto it = _channels.find(token.params[0]);

            if (it != _channels.end()) {
                it->second.setTopic(token.params[1]);
            } else {
                std::cerr << "No channel found with name: " << token.params[0]
                          << '\n';
            }
            break;
        }
        case IRCCommand::PART: {
            auto it = _channels.find(token.params[0]);

            if (it != _channels.end()) {
                it->second.removeUser(client);
                if (it->second.usersActive() == 0) {
                    _channels.erase(it->second.channelName());
                }
            } else {
                std::cerr << "No channel found with name: " << token.params[0]
                          << '\n';
            }
            break;
        }
        case IRCCommand::QUIT:
            _removeClient(client);
            break;
        case IRCCommand::PING:
            client->appendMessageToQue(_serverName, "PONG ", _serverName,
                                       " :" + token.params[0]);
            break;
        case IRCCommand::KICK: {
            auto it = _channels.find(token.params[0]);

            if (it != _channels.end()) {
                if (it->second.isOperator(client)) {
                    auto clientTarget = _nick_to_client.find(token.params[1]);
                    if (clientTarget != _nick_to_client.end()) {
                        it->second.removeUser(clientTarget->second);
                    } else {
                        std::cerr << "No user in the channel. error code??" << '\n';
                    }
                } else {
                    std::cerr << "Who was calling kick was not a operator" << '\n';
                }
            } else {
                std::cerr << "No channel with that name" << '\n';
            }
            break;
        }
        case IRCCommand::INVITE:
            std::cerr << "Not impl yet INVITE" << '\n';
            break;
        case IRCCommand::MODE_I:
            std::cerr << "Not impl yet MODE_I" << '\n';
            break;
        case IRCCommand::MODE_T:
            std::cerr << "Not impl yet MODE_T" << '\n';
            break;
        case IRCCommand::MODE_K:
            std::cerr << "Not impl yet MODE_K" << '\n';
            break;
        case IRCCommand::MODE_O:
            std::cerr << "Not impl yet MODE_O" << '\n';
            break;
        case IRCCommand::MODE_L:
            std::cerr << "Not impl yet MODE_L" << '\n';
            break;
        case IRCCommand::USERHOST: {
            auto it = _nick_to_client.find(token.params[0]);

            if (it != _nick_to_client.end()) {
                std::shared_ptr<Client> targetClient = it->second;
                std::string targetNick = targetClient->getNickname();

                client->appendMessageToQue(_serverName, "302 ", clientNick,
                                           " :", targetNick, "=-", targetNick,
                                           "@", targetClient->getIP());
            } else {
                std::cerr << "Server internal error: Could not found target "
                             "user for USERHOST"
                          << '\n';
            }
            break;
        }
        case IRCCommand::UNKNOW:
            std::cerr << "Not impl yet UNKNOW" << '\n';
            break;
    }
}
