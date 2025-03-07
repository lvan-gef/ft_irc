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
            if (token.success) {
                client->setNickname(token.params[0]);
                _clientAccepted(client);
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::USER:
            if (token.success) {
                client->setUsername(token.params[0]);
                _clientAccepted(client);
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::PASS:
            if (token.success) {
                client->setPasswordBit();
                _clientAccepted(client);
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::PRIVMSG:
            if (token.success) {
                std::cerr << "Not impl yet PRIV" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::JOIN:
            if (token.success) {
                std::cerr << "Not impl yet JOIN" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::PART:
            if (token.success) {
                std::cerr << "Not impl yet PART" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::QUIT:
            if (token.success) {
                std::cerr << "Not impl yet QUIT" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::PING:
            if (token.success) {
                _sendMessage(client->getFD(), " PONG ", _serverName,
                             " :" + token.params[0]);
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::KICK:
            if (token.success) {
                std::cerr << "Not impl yet KICK" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::INVITE:
            if (token.success) {
                std::cerr << "Not impl yet INVITE" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::MODE_I:
            if (token.success) {
                std::cerr << "Not impl yet MODE_I" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::MODE_T:
            if (token.success) {
                std::cerr << "Not impl yet MODE_T" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::MODE_K:
            if (token.success) {
                std::cerr << "Not impl yet MODE_K" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::MODE_O:
            if (token.success) {
                std::cerr << "Not impl yet MODE_O" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::MODE_L:
            if (token.success) {
                std::cerr << "Not impl yet MODE_L" << '\n';
            } else {
                _handleError(token, client);
            }
            break;
        case IRCCommand::UNKNOW:
            _handleError(token, client);
            break;
    }
}
