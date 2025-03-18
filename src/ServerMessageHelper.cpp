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
#include <memory>

#include "../include/Channel.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/utils.hpp"

void Server::_handleNickname(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {

    if (client->isRegistered()) {
        IRCMessage newToken = token;
        newToken.setIRCCode(IRCCodes::ALREADYREGISTERED);

        _handleError(newToken, client);
        return;
    }

    const std::string nickname = token.params[0];
    if (_nick_to_client.find(nickname) == _nick_to_client.end()) {
        client->setNickname(nickname);
        _clientAccepted(client);
    } else {
        IRCMessage newToken = token;
        newToken.setIRCCode(IRCCodes::NICKINUSE);

        _handleError(newToken, client);
    }
}

void Server::_handleUsername(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {
    if (client->isRegistered()) {
        IRCMessage newToken = token;
        newToken.setIRCCode(IRCCodes::ALREADYREGISTERED);

        _handleError(newToken, client);
    } else {
        client->setUsername(token.params[0]);
        _clientAccepted(client);
    }
}

void Server::_handlePassword(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {
    if (client->isRegistered()) {
        IRCMessage newToken = token;
        newToken.setIRCCode(IRCCodes::ALREADYREGISTERED);

        _handleError(newToken, client);
    } else {
        if (token.params[0] == _password) {
            client->setPasswordBit();
        } else {
            IRCMessage newToken = token;
            newToken.setIRCCode(IRCCodes::PASSWDMISMATCH);

            _handleError(newToken, client); // we should dissconnect the client
        }
    }
}

void Server::_handlePriv(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {
    const std::string nickname = token.params[0];

    if (nickname[0] == '#') {
        auto channel_it = _channels.find(nickname);

        if (channel_it != _channels.end()) {
            channel_it->second.broadcastMessage(token.params[1], nickname);
        } else {
            IRCMessage newToken = token;
            newToken.setIRCCode(IRCCodes::NOSUCHCHANNEL);

            _handleError(newToken, client);
        }
    } else {
        auto nick_it = _nick_to_client.find(nickname);
        if (nick_it != _nick_to_client.end()) {
            std::shared_ptr<Client> targetClient = nick_it->second;

            targetClient->appendMessageToQue(formatMessage(
                ":", nickname, " PRIVMSG ", targetClient->getNickname(), " :",
                token.params[1]));
        } else {
            IRCMessage newToken = token;
            newToken.setIRCCode(IRCCodes::NOSUCHNICK);

            _handleError(newToken, client);
        }
    }
}
