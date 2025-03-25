/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerMessageHelper.cpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/25 21:31:50 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include <memory>

#include "../include/Channel.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/utils.hpp"
#include "Optional.hpp"

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

    if (token.params[0][0] == '#' or token.params[0][0] == '&') {
        auto channel_it = _channels.find(token.params[0]);

        if (channel_it != _channels.end()) {
            channel_it->second.broadcast(
                client->getFullID(), "PRIVMSG " + channel_it->second.getName() +
                                         " :" + token.params[1]);
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
                          Channel(token.params[0], topic, client));
    } else {
        const std::string password =
            token.params.size() > 1 ? token.params[1] : "";
        IRCCode result = channel_it->second.addUser(password, client);

        if (result != IRCCode::SUCCES) {
            _handleError(formatError(token, result), client);
            return;
        }
    }

    channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
    }

    client->appendMessageToQue(formatMessage(
        ":", _serverName, " 332 ", client->getNickname(), " ",
        channel_it->second.getName(), " :", channel_it->second.getTopic()));
    client->appendMessageToQue(formatMessage(
        ":", _serverName, " 353 ", client->getNickname(), " = ",
        channel_it->second.getName(), " :", channel_it->second.getUserList()));
    client->appendMessageToQue(
        formatMessage(":", _serverName, " 366 ", client->getNickname(), " ",
                      channel_it->second.getName(), " :End of /NAMES list"));
}

// need to make a case to view the topic
void Server::_handleTopic(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);

    if (channel_it != _channels.end()) {
        if (token.params.size() < 2) {
            client->appendMessageToQue(
                formatMessage(":", _serverName, " 332 ", client->getNickname(),
                              " ", channel_it->second.getName(), " :",
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
        Channel channel = std::move(channel_it->second);

        IRCCode result = channel.removeUser(client);
        if (result != IRCCode::SUCCES) {
            return _handleError(formatError(token, result), client);
        }

        if (channel.isOperator(client) == true) {
            channel.broadcast(_serverName, " MODE " + channel.getName() +
                                               " -o " + client->getNickname());

            Optional<std::shared_ptr<Client>> newOperator =
                channel.removeOperator(client);
            if (newOperator.has_value()) {
                const std::shared_ptr<Client> &newClient =
                    newOperator.get_value();
                channel.broadcast(_serverName, " MODE " + channel.getName() +
                                                   " +o " +
                                                   newClient->getNickname());
            }
        }

        if (channel.activeUsers() == 0) {
            _channels.erase(channel.getName());
        }
    } else {
        return _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
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
    if (channel_it == _channels.end()) {
        return _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
    }

    auto userToKick_it = _nick_to_client.find(token.params[1]);
    if (userToKick_it == _nick_to_client.end()) {
        return _handleError(formatError(token, IRCCode::USERNOTINCHANNEL),
                            client);
    }

    IRCCode result = channel_it->second.kickUser(userToKick_it->second, client);
    if (result != IRCCode::SUCCES) {
        return _handleError(formatError(token, result), client);
    }
}

void Server::_handleInvite(const IRCMessage &token,
                           const std::shared_ptr<Client> &client) {

    auto channel_it = _channels.find(token.params[1]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    auto targetUser_it = _nick_to_client.find(token.params[0]);
    if (targetUser_it == _nick_to_client.end()) {
        _handleError(formatError(token, IRCCode::USERNOTINCHANNEL), client);
        return;
    }

    IRCCode result =
        channel_it->second.inviteUser(targetUser_it->second, client);
    if (result != IRCCode::SUCCES) {
        return _handleError(formatError(token, result), client);
    }

    targetUser_it->second->appendMessageToQue(formatMessage(
        ":", _serverName, " 332 ", targetUser_it->second->getNickname(), " ",
        channel_it->second.getName(), " :", channel_it->second.getTopic()));
    targetUser_it->second->appendMessageToQue(formatMessage(
        ":", _serverName, " 353 ", targetUser_it->second->getNickname(), " = ",
        channel_it->second.getName(), " :", channel_it->second.getUserList()));
    targetUser_it->second->appendMessageToQue(formatMessage(
        ":", _serverName, " 366 ", targetUser_it->second->getNickname(), " ",
        channel_it->second.getName(), " :End of /NAMES list"));
}

void Server::_handleModeI(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    /*IRCCode result = channel_it->second.modeI(token.params[1], client);*/
    /*if (result != IRCCode::SUCCES) {*/
    /*    _handleError(formatError(token, result), client);*/
    /*    return;*/
    /*}*/
}

void Server::_handleModeT(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    /*IRCCode result = channel_it->second.modeT(token.params[1], client);*/
    /*if (result != IRCCode::SUCCES) {*/
    /*    _handleError(formatError(token, result), client);*/
    /*    return;*/
    /*}*/
}

void Server::_handleModeK(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    /*IRCCode result =*/
    /*    channel_it->second.modeK(token.params[1], client, token.params[2]);*/
    /*if (result != IRCCode::SUCCES) {*/
    /*    _handleError(formatError(token, result), client);*/
    /*    return;*/
    /*}*/
}

void Server::_handleModeO(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        // check for code for user not on channel
        _handleError(formatError(token, IRCCode::CANNOTSENDTOCHAN), client);
        return;
    }

    auto user_it = _nick_to_client.find(token.params[2]);
    if (user_it == _nick_to_client.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHNICK), client);
        return;
    }

    /*IRCCode result =*/
    /*    channel_it->second.modeO(token.params[1], user_it->second, client);*/
    /*if (result != IRCCode::SUCCES) {*/
    /*    _handleError(formatError(token, result), client);*/
    /*    return;*/
    /*}*/
}

void Server::_handleModeL(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        _handleError(formatError(token, IRCCode::NOSUCHCHANNEL), client);
        return;
    }

    /*IRCCode result = channel_it->second.modeL(token.params[1],*/
    /*                                          toSizeT(token.params[2]),
     * client);*/
    /*if (result != IRCCode::SUCCES) {*/
    /*    _handleError(formatError(token, result), client);*/
    /*    return;*/
    /*}*/
}

static IRCMessage formatError(const IRCMessage &token, const IRCCode newCode) {
    IRCMessage newToken = token;

    newToken.setIRCCode(newCode);
    return newToken;
}
