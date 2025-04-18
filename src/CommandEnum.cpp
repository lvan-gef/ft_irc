/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CommandEnum.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/04/07 16:37:38 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <iostream>
#include <memory>

#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"

void Server::_handleCommand(const IRCMessage &token,
                            const std::shared_ptr<Client> &client) noexcept {

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
            _removeClient(client); // mark user for removeing§
            break;
        case IRCCommand::PING:
            return _handlePing(token, client);
        case IRCCommand::KICK: // send a message to operator what should not do
            return _handleKick(token, client);
        case IRCCommand::INVITE:
            return _handleInvite(token, client);
        case IRCCommand::MODE:
            return _handleMode(token, client);
        case IRCCommand::USERHOST:
            return _handleUserhost(token, client);
        case IRCCommand::UNKNOW:
            std::cerr << "Not impl yet UNKNOW" << '\n';
            break;
        default:
            std::cerr << "Unknow IRCCommand: " << token.command << '\n';
    }
}
