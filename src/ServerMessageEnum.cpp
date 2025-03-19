/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerMessageEnum.cpp                              :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/19 19:17:37 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <iostream>
#include <memory>

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
        case IRCCommand::JOIN:
            return _handleJoin(token, client);
        case IRCCommand::TOPIC:
            return _handleTopic(token, client);
        case IRCCommand::PART:
            return _handlePart(token, client);
        case IRCCommand::QUIT:
            _removeClient(client);  // mark user for removeing§
            break;
        case IRCCommand::PING:
            return _handlePing(token, client);
        case IRCCommand::KICK:
            return _handleKick(token, client);
        case IRCCommand::INVITE:
            return _handleInvite(token, client);
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
