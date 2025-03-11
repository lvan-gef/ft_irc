/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerMessageHelper.cpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/07 14:37:31 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <memory>

#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Channel.hpp"
#include "../include/utils.hpp"

void Server::_handleMessage(const IRCMessage &token,
                            const std::shared_ptr<Client> &client) {

    int clientFD = client->getFD();
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
            std::cerr << "Not impl yet PRIV" << '\n';
            break;
        case IRCCommand::CREATE:
            if (_channels.find(token.params[0]) == _channels.end()) {
                std::string topic = token.params.size() > 1 ? token.params[1] : "Default";
                _channels.emplace(token.params[0], Channel(_serverName, token.params[0], topic, client));
            } else {
                std::cerr << "Impl channel does exists" << '\n';
            }
            break;
        case IRCCommand::JOIN: {
            auto channel = _channels.find(token.params[0]);
            if (channel == _channels.end()) {
                std::string topic = token.params.size() > 1 ? token.params[1] : "Default";
                _channels.emplace(token.params[0], Channel(_serverName, token.params[0], topic, client));
            } else {
                channel->second.addUser(client);
            }
            break;
        }
        case IRCCommand::TOPIC:
            std::cerr << "Not impl yet TOPIC" << '\n';
            break;
        case IRCCommand::PART:
            std::cerr << "Not impl yet PART" << '\n';
            break;
        case IRCCommand::QUIT:
            std::cerr << "Not impl yet QUIT" << '\n';
            break;
        case IRCCommand::PING:
            sendMessage(clientFD, _serverName, " PONG ", _serverName,
                         " :" + token.params[0]);
            break;
        case IRCCommand::KICK:
            std::cerr << "Not impl yet KICK" << '\n';
            break;
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

                sendMessage(clientFD, _serverName, "302 ", clientNick, " :", targetNick,
                             "=-", targetNick, "@", targetClient->getIP());
            } else {
                std::cerr << "Server internal error: Could not found target "
                             "user for USERHOST"
                          << '\n';
            }

            break;
        }
        case IRCCommand::UNKNOW:
            break;
    }
}
