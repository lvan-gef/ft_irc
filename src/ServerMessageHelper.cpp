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

#include "../include/Server.hpp"
#include <iostream>
#include <memory>

void Server::_handleMessage(const IRCMessage &token,
                            const std::shared_ptr<Client> &client) {
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
        case IRCCommand::JOIN:
            std::cerr << "Not impl yet JOIN" << '\n';
            break;
        case IRCCommand::PART:
            std::cerr << "Not impl yet PART" << '\n';
            break;
        case IRCCommand::QUIT:
            std::cerr << "Not impl yet QUIT" << '\n';
            break;
        case IRCCommand::PING:
            _sendMessage(client->getFD(), " PONG ", _serverName,
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
        case IRCCommand::UNKNOW:
            break;
    }
}
