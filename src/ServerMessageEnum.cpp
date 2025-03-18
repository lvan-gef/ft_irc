/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerMessageHelper.cpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/17 21:30:28 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <iostream>
#include <memory>

#include "../include/Channel.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/utils.hpp"

void Server::_handleMessage(const IRCMessage &token,
                            const std::shared_ptr<Client> &client) {

    std::string clientNick = client->getNickname();

    switch (token.type) {
        case IRCCommand::NICK:
            return _handleNickname(token, client);
        case IRCCommand::USER:
            return _handleUsername(token, client);
        case IRCCommand::PASS:
            return _handlePassword(token, client);
        case IRCCommand::PRIVMSG:
            return _handlePriv(token, client);
        case IRCCommand::JOIN: {
            auto channel_it = _channels.find(token.params[0]);
            if (channel_it == _channels.end()) {
                std::string topic =
                    token.params.size() > 1 ? token.params[1] : "Default";
                _channels.emplace(
                    token.params[0],
                    Channel(_serverName, token.params[0], topic, client));
            } else {
                if (channel_it->second.inviteOnly()) {
                    IRCMessage newToken = token;
                    newToken.setIRCCode(IRCCodes::INVITEONLYCHAN);

                    _handleError(token, client);
                } else {
                    IRCCodes addResult = channel_it->second.addUser(client);
                    if (addResult != IRCCodes::SUCCES) {
                        IRCMessage newToken = token;
                        newToken.setIRCCode(addResult);

                        _handleError(token, client);
                    }
                }
            }
            break;
        }
        case IRCCommand::TOPIC: {
            auto it = _channels.find(token.params[0]);

            if (it != _channels.end()) {
                it->second.setTopic(token.params[1]);
            } else {
                IRCMessage newToken = token;
                newToken.setIRCCode(IRCCodes::NOSUCHCHANNEL);

                _handleError(newToken, client);
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
                IRCMessage newToken = token;
                newToken.setIRCCode(IRCCodes::NOSUCHCHANNEL);

                _handleError(newToken, client);
            }
            break;
        }
        case IRCCommand::QUIT:
            _removeClient(client);
            break;
        case IRCCommand::PING:
            client->appendMessageToQue(formatMessage(":", _serverName, " PONG ",
                                                     _serverName,
                                                     " :" + token.params[0]));
            break;
        case IRCCommand::KICK: {
            auto it = _channels.find(token.params[0]);

            if (it != _channels.end()) {
                if (it->second.isOperator(client)) {
                    auto clientTarget = _nick_to_client.find(token.params[1]);
                    if (clientTarget != _nick_to_client.end()) {
                        it->second.removeUser(clientTarget->second);
                    } else {
                        std::cerr << "No user in the channel. error code??"
                                  << '\n';
                    }
                } else {
                    std::cerr << "Who was calling kick was not a operator"
                              << '\n';
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

                client->appendMessageToQue(formatMessage(
                    ":", _serverName, " 302 ", clientNick, " :", targetNick,
                    "=-", targetNick, "@", targetClient->getIP()));
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
