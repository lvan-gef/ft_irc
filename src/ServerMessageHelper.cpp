/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerMessageHelper.cpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/19 20:50:09 by lvan-gef      ########   odam.nl         */
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

static IRCMessage formatError(const IRCMessage &token, const IRCCode newCode);

void Server::_handleNickname(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {

    // sould also handle setting of nickname when user is on server
    if (client->isRegistered()) {
        _handleError(formatError(token, IRCCode::ALREADYREGISTERED), client);
        return;
    }

    const std::string nickname = token.params[0];
    if (_nick_to_client.find(nickname) == _nick_to_client.end()) {
        client->setNickname(nickname);
        _clientAccepted(client);
    } else {
        _handleError(formatError(token, IRCCode::NICKINUSE), client);
    }
}

void Server::_handleUsername(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {
    if (client->isRegistered()) {
        _handleError(formatError(token, IRCCode::ALREADYREGISTERED), client);
    } else {
        client->setUsername(token.params[0]);
        _clientAccepted(client);
    }
}

void Server::_handlePassword(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {
    if (client->isRegistered()) {
        _handleError(formatError(token, IRCCode::ALREADYREGISTERED), client);
    } else {
        if (token.params[0] == _password) {
            client->setPasswordBit();
        } else {
            // we should dissconnect the client
            _handleError(formatError(token, IRCCode::PASSWDMISMATCH), client);
        }
    }
}

void Server::_handlePriv(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {

    if (token.params[0][0] == '#') {
        auto channel_it = _channels.find(token.params[0]);

        if (channel_it != _channels.end()) {
            channel_it->second.sendMessage(" :" + token.params[1],
                                           client->getFullID());
        } else {
            _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        }
    } else {
        auto nick_it = _nick_to_client.find(token.params[0]);

        if (nick_it != _nick_to_client.end()) {
            std::shared_ptr<Client> targetClient = nick_it->second;

            targetClient->appendMessageToQue(formatMessage(
                ":", client->getFullID(), " PRIVMSG ",
                targetClient->getNickname(), " :", token.params[1]));
        } else {
            _handleError(formatError(token, IRCCode::NOSUCHNICK), client);
        }
    }
}

void Server::_handleJoin(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);

    if (channel_it == _channels.end()) {
        std::string topic =
            token.params.size() > 1 ? token.params[1] : "Default";
        _channels.emplace(token.params[0],
                          Channel(_serverName, token.params[0], topic, client));
    } else {
        const std::string password =
            token.params.size() > 1 ? token.params[1] : "";
        IRCCode result = channel_it->second.addUser(password, client);

        if (result != IRCCode::SUCCES) {
            _handleError(formatError(token, result), client);
        }
    }
}

// need to make a case to view the topic
void Server::_handleTopic(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);

    if (channel_it != _channels.end()) {
        if (token.params.size() < 2) {
            client->appendMessageToQue(
                formatMessage(":", _serverName, " 332 ", client->getNickname(),
                              " ", channel_it->second.channelName(), " :",
                              channel_it->second.getTopic()));
            return;
        }

        IRCCode result = channel_it->second.setTopic(token.params[1], client);
        if (result != IRCCode::SUCCES) {
            _handleError(formatError(token, result), client);
        }
    } else {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
    }
}

void Server::_handlePart(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);

    if (channel_it != _channels.end()) {
        channel_it->second.removeUser(client);
        if (channel_it->second.usersActive() == 0) {
            _channels.erase(channel_it->second.channelName());
        }
    } else {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
    }
}

void Server::_handlePing(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {
    client->appendMessageToQue(formatMessage(
        ":", _serverName, " PONG ", _serverName, " :" + token.params[0]));
}

void Server::_handleKick(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);

    if (channel_it != _channels.end()) {
        auto userToKick = _nick_to_client.find(token.params[1]);
        if (userToKick != _nick_to_client.end()) {
            IRCCode result =
                channel_it->second.kickUser(userToKick->second, client);

            if (result != IRCCode::SUCCES) {
                if (result == IRCCode::UNKNOWNCOMMAND) {
                    // when it trys to kick himself
                    // does not show up in channel, only in global
                    client->appendMessageToQue(formatMessage(
                        _serverName, " NOTICE ", client->getNickname(), " ",
                        channel_it->second.channelName(),
                        " :You cannot kick yourself from the channel"));
                    return;
                }
                _handleError(formatError(token, result), client);
            }
        } else {
            _handleError(formatError(token, IRCCode::USERNOTINCHANNEL),
                         client);
        }
    } else {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
    }
}

void Server::_handleInvite(const IRCMessage &token,
                           const std::shared_ptr<Client> &client) {

    auto targetUser_it = _nick_to_client.find(token.params[0]);
    if (targetUser_it == _nick_to_client.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHNICK), client);
        return;
    }

    auto channel_it = _channels.find(token.params[1]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    IRCCode result =
        channel_it->second.inviteUser(targetUser_it->second, client);
    if (result != IRCCode::SUCCES) {
        _handleError(formatError(token, result), client);
    }
}

void Server::_handleModeI(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    IRCCode result = channel_it->second.modeI(token.params[1], client);
    if (result != IRCCode::SUCCES) {
        _handleError(formatError(token, result), client);
        return;
    }
}

void Server::_handleModeT(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    IRCCode result = channel_it->second.modeT(token.params[1], client);
    if (result != IRCCode::SUCCES) {
        _handleError(formatError(token, result), client);
        return;
    }
}

void Server::_handleModeK(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    IRCCode result = channel_it->second.modeK(token.params[1], client, token.params[2]);
    if (result != IRCCode::SUCCES) {
        _handleError(formatError(token, result), client);
        return;
    }
}

static IRCMessage formatError(const IRCMessage &token, const IRCCode newCode) {
    IRCMessage newToken = token;

    newToken.setIRCCode(newCode);
    return newToken;
}
