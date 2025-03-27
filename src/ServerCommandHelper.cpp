/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerCommandHelper.cpp                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/27 16:36:53 by lvan-gef      ########   odam.nl         */
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

static IRCMessage setCode(const IRCMessage &token, const IRCCode newCode);

void Server::_handleNickname(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {

    const std::string nickname = token.params[0];
    if (_nick_to_client.find(nickname) != _nick_to_client.end()) {
        return handleMsg(setCode(token, IRCCode::NICKINUSE), client, "", "");
    }

    std::string old_id = client->getFullID();
    std::string old_nickname = client->getNickname();
    client->setNickname(nickname);

    if (client->isRegistered() != true) {
        _clientAccepted(client);
    } else {
        client->appendMessageToQue(
            formatMessage(":", old_id, " NICK ", client->getNickname()));

        for (const std::string &channelName : client->allChannels()) {
            auto it = _channels.find(channelName);
            if (it != _channels.end()) {
                it->second.broadcast(old_id, " NICK " + client->getNickname());
                break;
            }
        }
    }

    _nick_to_client[client->getNickname()] = client;
    _nick_to_client.erase(old_nickname);
}

void Server::_handleUsername(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {
    if (client->isRegistered()) {
        handleMsg(setCode(token, IRCCode::ALREADYREGISTERED), client, "", "");
    } else {
        client->setUsername(token.params[0]);
        _clientAccepted(client);
    }
}

void Server::_handlePassword(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {
    if (client->isRegistered()) {
        handleMsg(setCode(token, IRCCode::ALREADYREGISTERED), client, "", "");
    } else {
        if (token.params[0] == _password) {
            client->setPasswordBit();
        } else {
            // we should dissconnect the client
            handleMsg(setCode(token, IRCCode::PASSWDMISMATCH), client, "", "");
        }
    }
}

void Server::_handlePriv(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {

    if (token.params[0][0] == '#') {
        auto channel_it = _channels.find(token.params[0]);
        if (channel_it == _channels.end()) {
            return handleMsg(setCode(token, IRCCode::NOSUCHCHANNEL), client, "",
                             "");
        }

        channel_it->second.broadcast(client->getFullID(),
                                     "PRIVMSG " + channel_it->second.getName() +
                                         " :" + token.params[1]);
    } else {
        auto nick_it = _nick_to_client.find(token.params[0]);
        if (nick_it == _nick_to_client.end()) {
            return handleMsg(setCode(token, IRCCode::NOSUCHNICK), client, "",
                             "");
        }

        nick_it->second->appendMessageToQue(formatMessage(
            ":", client->getFullID(), " PRIVMSG ",
            nick_it->second->getNickname(), " :", token.params[1]));
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
            return handleMsg(setCode(token, result), client, "", "");
        }
    }

    channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return handleMsg(setCode(token, IRCCode::NOSUCHCHANNEL), client, "",
                         "");
    }

    handleMsg(setCode(token, IRCCode::TOPIC), client,
              channel_it->second.getName(), channel_it->second.getTopic());
    handleMsg(setCode(token, IRCCode::NAMREPLY), client,
              channel_it->second.getName(), channel_it->second.getUserList());
    handleMsg(setCode(token, IRCCode::ENDOFNAMES), client,
              channel_it->second.getName(), "");
    client->addChannel(channel_it->second.getName());
}

void Server::_handleTopic(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return handleMsg(setCode(token, IRCCode::NOSUCHCHANNEL), client, "",
                         "");
    }

    if (token.params.size() < 2) {
        handleMsg(setCode(token, IRCCode::TOPIC), client,
                  channel_it->second.getName(), channel_it->second.getTopic());
        return;
    }

    IRCCode result = channel_it->second.setTopic(token.params[1], client);
    if (result != IRCCode::SUCCES) {
        return handleMsg(setCode(token, result), client, "", "");
    }

    handleMsg(setCode(token, IRCCode::TOPIC), client,
              channel_it->second.getName(), channel_it->second.getTopic());
}

void Server::_handlePart(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);

    if (channel_it == _channels.end()) {
        return handleMsg(setCode(token, IRCCode::NOSUCHCHANNEL), client, "",
                         "");
    }

    IRCCode result = channel_it->second.removeUser(client);
    if (result != IRCCode::SUCCES) {
        return handleMsg(setCode(token, result), client, "", "");
    }

    if (channel_it->second.getActiveUsers() == 0) {
        _channels.erase(channel_it->second.getName());
    }
}

void Server::_handlePing(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {
    client->appendMessageToQue(formatMessage(
        ":", serverName, " PONG ", serverName, " :" + token.params[0]));
}

void Server::_handleKick(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return handleMsg(setCode(token, IRCCode::NOSUCHCHANNEL), client, "",
                         "");
    }

    auto userToKick_it = _nick_to_client.find(token.params[1]);
    if (userToKick_it == _nick_to_client.end()) {
        return handleMsg(setCode(token, IRCCode::USERNOTINCHANNEL), client, "",
                         "");
    }

    IRCCode result = channel_it->second.kickUser(userToKick_it->second, client);
    if (result != IRCCode::SUCCES) {
        return handleMsg(setCode(token, result), client, "", "");
    }
}

void Server::_handleInvite(const IRCMessage &token,
                           const std::shared_ptr<Client> &client) {

    auto channel_it = _channels.find(token.params[1]);
    if (channel_it == _channels.end()) {
        handleMsg(setCode(token, IRCCode::NOSUCHCHANNEL), client, "", "");
        return;
    }

    auto targetUser_it = _nick_to_client.find(token.params[0]);
    if (targetUser_it == _nick_to_client.end()) {
        handleMsg(setCode(token, IRCCode::NOSUCHNICK), client, "", "");
        return;
    }

    IRCCode result =
        channel_it->second.inviteUser(targetUser_it->second, client);
    if (result != IRCCode::SUCCES) {
        return handleMsg(setCode(token, result), client, "", "");
    }

    handleMsg(setCode(token, IRCCode::TOPIC), client,
              channel_it->second.getName(), channel_it->second.getTopic());
    handleMsg(setCode(token, IRCCode::NAMREPLY), client,
              channel_it->second.getName(), channel_it->second.getUserList());
    handleMsg(setCode(token, IRCCode::ENDOFNAMES), client,
              channel_it->second.getName(), "");
}

void Server::_handleMode(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) {

    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return handleMsg(setCode(token, IRCCode::NOSUCHCHANNEL), client, "",
                         "");
    }

    if (token.params.size() < 2) {
        handleMsg(setCode(token, IRCCode::CHANNELMODEIS), client,
                  channel_it->second.getName(),
                  channel_it->second.getModes() +
                      channel_it->second.getModesValues());
        return;
    }

    const bool state = true ? token.params[1][0] == '+' : false;
    const ChannelCommand cmd = static_cast<ChannelCommand>(token.params[1][1]);
    const std::string value = token.params.size() > 2 ? token.params[2] : "";
    IRCCode result = {};
    std::string suffix = "";

    switch (cmd) {
        case ChannelCommand::MODE_I:
            result = channel_it->second.setMode(ChannelMode::INVITE_ONLY, state,
                                                value, client);
            break;
        case ChannelCommand::MODE_T:
            result = channel_it->second.setMode(ChannelMode::TOPIC_PROTECTED,
                                                state, value, client);
            break;
        case ChannelCommand::MODE_K:
            result = channel_it->second.setMode(ChannelMode::PASSWORD_PROTECTED,
                                                state, value, client);
            break;
        case ChannelCommand::MODE_O:
            result = channel_it->second.setMode(ChannelMode::OPERATOR, state,
                                                value, client);
            suffix = value;
            break;
        case ChannelCommand::MODE_L:
            result = channel_it->second.setMode(ChannelMode::USER_LIMIT, state,
                                                value, client);
            suffix = value;
            break;
        default:
            std::cerr << "Unknow channel mode: " << token.params[1][1] << '\n';
            return;
    }

    if (result != IRCCode::SUCCES) {
        return handleMsg(setCode(token, result), client, "", "");
    }

    channel_it->second.broadcast(serverName,
                                 "MODE " + channel_it->second.getName() + " " +
                                     token.params[1] + " " + suffix);
}

void Server::_handleUserhost(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) {
    auto it = _nick_to_client.find(token.params[0]);
    if (it == _nick_to_client.end()) {
        std::cerr << "Server internal error: Could not found target "
                     "user for USERHOST"
                  << '\n';
        return;
    }

    std::shared_ptr<Client> targetClient = it->second;
    std::string targetNick = targetClient->getNickname();
    handleMsg(setCode(token, IRCCode::USERHOST), client, "",
              targetNick + "=-" + client->getFullID());
}

static IRCMessage setCode(const IRCMessage &token, const IRCCode newCode) {
    IRCMessage newToken = token;

    newToken.setIRCCode(newCode);
    return newToken;
}
