/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CommandHelper.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 14:37:31 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/04/22 20:41:52 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <memory>

#include "../include/Channel.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/Utils.hpp"

void Server::_handleNickname(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) noexcept {

    const std::string nickname = token.params[0];
    if (_nick_to_client.find(nickname) != _nick_to_client.end()) {
        return handleMsg(IRCCode::NICKINUSE, client, token.params[0], "");
    }

    std::string old_nickname = client->getNickname();
    client->setNickname(nickname);

    if (client->isRegistered() != true) {
        _clientAccepted(client);
    } else {
        std::string old_id = client->getFullID();
        handleMsg(IRCCode::NICKCHANGED, client, old_id, client->getNickname());

        for (const std::string &channelName : client->allChannels()) {
            auto it = _channels.find(channelName);
            if (it != _channels.end()) {
                it->second.broadcast(IRCCode::NICKCHANGED, old_id,
                                     client->getNickname());
                break;
            }
        }
    }

    _nick_to_client[client->getNickname()] = client;
    _nick_to_client.erase(old_nickname);
}

void Server::_handleUsername(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) noexcept {
    if (client->isRegistered()) {
        handleMsg(IRCCode::ALREADYREGISTERED, client, "", "");
    } else {
        client->setUsername(token.params[0]);
        _clientAccepted(client);
    }
}

void Server::_handlePassword(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) noexcept {
    if (client->isRegistered()) {
        handleMsg(IRCCode::ALREADYREGISTERED, client, "", "");
    } else {
        if (token.params[0] == _password) {
            client->setPasswordBit();
        } else {
            client->setDisconnect();
            handleMsg(IRCCode::PASSWDMISMATCH, client, "", "");
        }
    }
}

void Server::_handlePriv(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {

    if (token.params[0][0] == '#') {
        auto channel_it = _channels.find(token.params[0]);
        if (channel_it == _channels.end()) {
            return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0],
                             "");
        }
        channel_it->second.broadcast(IRCCode::PRIVMSG, client->getFullID(),
                                     ":" + token.params[1]);
    } else {
        auto nick_it = _nick_to_client.find(token.params[0]);
        if (nick_it == _nick_to_client.end()) {
            return handleMsg(IRCCode::NOSUCHNICK, client, token.params[0], "");
        }
        handleMsg(IRCCode::PRIVMSG, nick_it->second, client->getFullID(),
                  client->getNickname() + " :" + token.params[1]);
    }
}

void Server::_handleJoin(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    auto channel_it = _channels.find(token.params[0]);

    if (channel_it == _channels.end()) {
        std::string topic =
            token.params.size() > 1 ? token.params[1] : "Default";
        _channels.emplace(token.params[0],
                          Channel(token.params[0], topic, client));
    } else {
        const std::string password =
            token.params.size() > 1 ? token.params[1] : "";
        if (channel_it->second.addUser(password, client) != true) {
            return;
        }
    }

    channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0], "");
    }

    channel_it->second.setTopic("Default", client);
    handleMsg(IRCCode::NAMREPLY, client, channel_it->second.getName(),
              channel_it->second.getUserList());
    handleMsg(IRCCode::ENDOFNAMES, client, channel_it->second.getName(), "");
    client->addChannel(channel_it->second.getName());
}

void Server::_handleTopic(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) noexcept {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0], "");
    }

    if (token.params.size() < 2) {
        handleMsg(IRCCode::TOPIC, client, "",
                  channel_it->second.getName() + " :" +
                      channel_it->second.getTopic());
        return;
    }

    channel_it->second.setTopic(token.params[1], client);
}

void Server::_handlePart(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    auto channel_it = _channels.find(token.params[0]);

    if (channel_it == _channels.end()) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0], "");
    }

    const std::string reason = token.params.size() > 1 ? token.params[1] : "";
    channel_it->second.removeUser(client, reason);
    if (channel_it->second.getActiveUsers() == 0) {
        _channels.erase(channel_it->second.getName());
    }
}

void Server::_handlePing(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    client->appendMessageToQue(formatMessage(
        ":", serverName, " PONG ", serverName, " :" + token.params[0]));
}

void Server::_handleKick(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0], "");
    }

    auto userToKick_it = _nick_to_client.find(token.params[1]);
    if (userToKick_it == _nick_to_client.end()) {
        return handleMsg(IRCCode::USERNOTINCHANNEL, client, token.params[1],
                         "");
    }

    const std::string rsn = token.params.size() > 2 ? token.params[2] : "bye";
    channel_it->second.kickUser(userToKick_it->second, client, rsn);
}

void Server::_handleInvite(const IRCMessage &token,
                           const std::shared_ptr<Client> &client) noexcept {
    auto channel_it = _channels.find(token.params[1]);
    if (channel_it == _channels.end()) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0], "");
    }

    auto targetUser_it = _nick_to_client.find(token.params[0]);
    if (targetUser_it == _nick_to_client.end()) {
        handleMsg(IRCCode::NOSUCHNICK, client, token.params[0], "");
        return;
    }

    if (channel_it->second.inviteUser(targetUser_it->second, client)) {
        handleMsg(IRCCode::TOPIC, targetUser_it->second,
                  channel_it->second.getName(), channel_it->second.getTopic());
        handleMsg(IRCCode::NAMREPLY, targetUser_it->second,
                  channel_it->second.getName(),
                  channel_it->second.getUserList());
        handleMsg(IRCCode::ENDOFNAMES, targetUser_it->second,
                  channel_it->second.getName(), "");
    }
}

void Server::_handleMode(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {

    auto channel_it = _channels.find(token.params[0]);
    if (channel_it == _channels.end()) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0], "");
    }

    if (token.params.size() < 2) {
        return handleMsg(IRCCode::CHANNELMODEIS, client,
                         channel_it->second.getName(),
                         channel_it->second.getModes() +
                             channel_it->second.getModesValues());
    }

    const bool state = true ? token.params[1][0] == '+' : false;
    const ChannelCommand cmd = static_cast<ChannelCommand>(token.params[1][1]);
    const std::string value = token.params.size() > 2 ? token.params[2] : "";

    switch (cmd) {
        case ChannelCommand::MODE_I:
            channel_it->second.setMode(ChannelMode::INVITE_ONLY, state, value,
                                       client);
            break;
        case ChannelCommand::MODE_T:
            channel_it->second.setMode(ChannelMode::TOPIC_PROTECTED, state,
                                       value, client);
            break;
        case ChannelCommand::MODE_K:
            channel_it->second.setMode(ChannelMode::PASSWORD_PROTECTED, state,
                                       value, client);
            break;
        case ChannelCommand::MODE_O:
            channel_it->second.setMode(ChannelMode::OPERATOR, state, value,
                                       client);
            break;
        case ChannelCommand::MODE_L:
            channel_it->second.setMode(ChannelMode::USER_LIMIT, state, value,
                                       client);
            break;
    }
}

void Server::_handleUserhost(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) noexcept {
    auto it = _nick_to_client.find(token.params[0]);
    if (it == _nick_to_client.end()) {
        std::cerr << "Server internal error: Could not found target "
                     "user for USERHOST"
                  << '\n';
        return;
    }

    std::shared_ptr<Client> targetClient = it->second;
    std::string targetNick = targetClient->getNickname();
    handleMsg(IRCCode::USERHOST, client, "",
              targetNick + "=-" + client->getFullID());
}
