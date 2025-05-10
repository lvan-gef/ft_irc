#include <algorithm>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "../include/Channel.hpp"
#include "../include/Chatbot.hpp"
#include "../include/Client.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/Utils.hpp"

void Server::_handleNickname(const IRCMessage &token,
                             const std::shared_ptr<Client> &client) noexcept {
    std::string nickname = token.params[0];
    std::transform(nickname.begin(), nickname.end(), nickname.begin(),
                   ::toupper);

    for (const auto &it : _nick_to_client) {
        std::string uppercaseIt = it.first;
        std::transform(uppercaseIt.begin(), uppercaseIt.end(),
                       uppercaseIt.begin(), ::toupper);
        if (uppercaseIt == nickname) {
            return handleMsg(IRCCode::NICKINUSE, client, token.params[0], "");
        }
    }

    const bool wasRegistered = client->isRegistered();
    const std::string old_id = client->getFullID();
    const std::string old_nickname = client->getNickname();
    client->setNickname(token.params[0]);

    if (!wasRegistered && client->isRegistered()) {
        _clientAccepted(client);
    } else if (client->isRegistered()) {
        handleMsg(IRCCode::NICKCHANGED, client, old_id, client->getNickname());

        for (const std::string &channelName : client->allChannels()) {
            std::string cn = channelName;
            std::transform(cn.begin(), cn.end(), cn.begin(), ::toupper);
            auto it = _channels.find(cn);
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
        client->setRealname(token.params[3]);
        _clientAccepted(client);
    }
}

void Server::_handlePassword(
    const IRCMessage &token,
    const std::shared_ptr<Client> &client) const noexcept {
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
    if (token.params[1].empty()) {
        return handleMsg(IRCCode::NOTEXTTOSEND, client, client->getNickname(),
                         "");
    }

    std::string bot = token.params[0];
    std::transform(bot.begin(), bot.end(), bot.begin(), ::toupper);

    const std::string channelName = token.params[0];
    const Channel *channel = isChannel(channelName);
    if (token.params[0][0] == '#') {
        if (channel == nullptr) {
            return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0],
                             "");
        }

        bot = token.params[1];
        std::transform(bot.begin(), bot.end(), bot.begin(), ::toupper);
        if (bot == "!BOT") {
            handleMsg(
                IRCCode::PRIVMSG, client, ("Bot!Bot@codamirc.local"),
                client->getNickname() +
                    "Hello, I am bot. I can say hello, check weather and send "
                    "PONG back, send a quote and print channels and users.");
        } else {
            if (channel->userOnChannel(client) != true) {
                return handleMsg(IRCCode::USERNOTINCHANNEL, client,
                                 channel->getName(), client->getNickname());
            }

            channel->broadcast(IRCCode::PRIVMSG, client->getFullID(),
                               ":" + token.params[1]);
        }
    } else if (bot == "BOT") {
        const std::string response = handleBot(token.params, client, this);
        const std::size_t find = response.find('\n');
        if (std::string::npos != find) {
            botResponseNl(client, response);
        } else {
            handleMsg(IRCCode::PRIVMSG, client, ("Bot!Bot@codamirc.local"),
                      client->getNickname() + " :" + response);
        }
    } else {
        std::string nickname = token.params[0];
        std::transform(nickname.begin(), nickname.end(), nickname.begin(),
                       ::toupper);

        for (auto const &it : _nick_to_client) {
            std::string upperCaseIt = it.second->getNickname();
            std::transform(upperCaseIt.begin(), upperCaseIt.end(),
                           upperCaseIt.begin(), ::toupper);
            if (upperCaseIt == nickname) {
                return handleMsg(
                    IRCCode::PRIVMSG, it.second, client->getFullID(),
                    client->getNickname() + " :" + token.params[1]);
            }
        }
        return handleMsg(IRCCode::NOSUCHNICK, client, token.params[0], "");
    }
}

void Server::_handleJoin(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    for (std::size_t index = 0; index < token.params.size(); ++index) {
        const std::string &channelName = token.params[index];

        Channel *channel = isChannel(channelName);
        if (channel == nullptr) {
            const auto emplace_result = _channels.emplace(
                channelName, Channel(channelName, "Default", client));
            if (!emplace_result.second) {
                handleMsg(IRCCode::NOSUCHCHANNEL, client, channelName, "");
                continue;
            }

            channel = &(emplace_result.first->second);
        } else {
            const std::string &password =
                index < token.keys.size() ? token.keys[index] : "";
            if (!channel->addUser(password, client)) {
                continue;
            }
        }

        if (channel->hasInvite()) {
            if (!channel->isInvited(client)) {
                handleMsg(IRCCode::INVITEONLYCHAN, client, channel->getName(),
                          "");

                continue;
            }
        }

        handleMsg(IRCCode::TOPIC, client, channel->getName(),
                  channel->getTopic());
        handleMsg(IRCCode::NAMREPLY, client, channel->getName(),
                  channel->getUserList());
        handleMsg(IRCCode::ENDOFNAMES, client, channel->getName(), "");
        client->addChannel(channel->getName());
        channel->removeFromInvited(client);
    }
}

void Server::_handleTopic(const IRCMessage &token,
                          const std::shared_ptr<Client> &client) noexcept {
    const std::string channelName = token.params[0];

    Channel *channel = isChannel(channelName);
    if (channel == nullptr) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0], "");
    }

    if (token.params.size() < 2) {
        handleMsg(IRCCode::TOPIC, client, "",
                  channel->getName() + " :" + channel->getTopic());
        return;
    }

    channel->setTopic(token.params[1], client);
}

void Server::_handlePart(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    for (const std::string &channelName : token.params) {
        Channel *channel = isChannel(channelName);

        if (channel == nullptr) {
            return handleMsg(IRCCode::NOSUCHCHANNEL, client, channelName, "");
        }

        channel->removeUser(client, token.reason);
    }
}

void Server::_handlePing(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    client->appendMessageToQue(formatMessage(
        ":", serverName, " PONG ", serverName, " :" + token.params[0]));
}

void Server::_handleKick(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    for (const std::string &channelName : token.params) {

        Channel *channel = isChannel(channelName);
        if (channel == nullptr) {
            handleMsg(IRCCode::NOSUCHCHANNEL, client, channelName, "");
            continue;
        }

        std::shared_ptr<Client> targetClient = nullptr;
        for (const std::string &username : token.keys) {
            std::string upperUserNickToKick = username;
            std::transform(upperUserNickToKick.begin(),
                           upperUserNickToKick.end(),
                           upperUserNickToKick.begin(), ::toupper);

            for (const auto &pair : _nick_to_client) {
                std::string currentNickUpper = pair.first;
                std::transform(currentNickUpper.begin(), currentNickUpper.end(),
                               currentNickUpper.begin(), ::toupper);
                if (currentNickUpper == upperUserNickToKick) {
                    targetClient = pair.second;
                    break;
                }
            }

            if (targetClient == nullptr) {
                handleMsg(IRCCode::NOSUCHNICK, client, username, "");
                continue;
            }

            channel->kickUser(targetClient, client, token.reason);
        }
    }
}

void Server::_handleInvite(const IRCMessage &token,
                           const std::shared_ptr<Client> &client) noexcept {
    const std::string channelName = token.params[1];

    std::shared_ptr<Client> targetClient = nullptr;
    Channel *channel = isChannel(channelName);
    if (channel == nullptr) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[1], "");
    }

    std::string nickname = token.params[0];
    std::transform(nickname.begin(), nickname.end(), nickname.begin(),
                   ::toupper);

    for (auto const &it : _nick_to_client) {
        std::string upperCaseIt = it.second->getNickname();
        std::transform(upperCaseIt.begin(), upperCaseIt.end(),
                       upperCaseIt.begin(), ::toupper);
        if (upperCaseIt == nickname) {
            targetClient = it.second;
            break;
        }
    }

    if (targetClient == nullptr) {
        return handleMsg(IRCCode::NOSUCHNICK, client, token.params[0], "");
    }

    channel->inviteUser(targetClient, client);
}

void Server::_handleMode(const IRCMessage &token,
                         const std::shared_ptr<Client> &client) noexcept {
    const std::string channelName = token.params[0];

    Channel *channel = isChannel(channelName);
    if (channel == nullptr) {
        return handleMsg(IRCCode::NOSUCHCHANNEL, client, token.params[0], "");
    }

    if (token.params.size() < 2) {
        return handleMsg(IRCCode::CHANNELMODEIS, client, channel->getName(),
                         channel->getChannelModes() +
                             channel->getChannelModesValues());
    }

    const bool state = token.params[1][0] == '+';
    const auto cmd = static_cast<ChannelCommand>(token.params[1][1]);
    const std::string value = token.params.size() > 2 ? token.params[2] : "";

    switch (cmd) {
        case ChannelCommand::MODE_I:
            channel->setMode(ChannelMode::INVITE_ONLY, state, value, client);
            break;
        case ChannelCommand::MODE_T:
            channel->setMode(ChannelMode::TOPIC_PROTECTED, state, value,
                             client);
            break;
        case ChannelCommand::MODE_K:
            channel->setMode(ChannelMode::PASSWORD_PROTECTED, state, value,
                             client);
            break;
        case ChannelCommand::MODE_O:
            channel->setMode(ChannelMode::OPERATOR, state, value, client);
            break;
        case ChannelCommand::MODE_L:
            channel->setMode(ChannelMode::USER_LIMIT, state, value, client);
            break;
    }

    return channel->broadcast(IRCCode::CHANNELMODEIS, "",
                              channel->getChannelModes() +
                                  channel->getChannelModesValues());
}

void Server::_handleUserhost(
    const IRCMessage &token,
    const std::shared_ptr<Client> &client) const noexcept {
    std::shared_ptr<Client> targetClient = nullptr;
    std::string uppercaseName = token.params[0];
    std::transform(uppercaseName.begin(), uppercaseName.end(),
                   uppercaseName.begin(), ::toupper);

    for (auto const &it : _nick_to_client) {
        std::string upperCaseIt = it.second->getNickname();
        std::transform(upperCaseIt.begin(), upperCaseIt.end(),
                       upperCaseIt.begin(), ::toupper);
        if (upperCaseIt == uppercaseName) {
            targetClient = it.second;
            break;
        }
    }

    if (targetClient == nullptr) {
        std::cerr << "Server internal error: Could not found target "
                     "user for USERHOST"
                  << '\n';
        return;
    }

    const std::string targetNick = targetClient->getNickname();
    handleMsg(IRCCode::USERHOST, client, "",
              targetNick + "=-" + client->getFullID());
}

void Server::_handleUnkown(const IRCMessage &token,
                           const std::shared_ptr<Client> &client) noexcept {
    return handleMsg(IRCCode::UNKNOWNCOMMAND, client, token.command,
                     "Unknown command");
}

void Server::_handleWhois(
    const IRCMessage &token,
    const std::shared_ptr<Client> &client) const noexcept {
    if (token.params.empty())
        return;

    std::string upperCaseNick = token.params[0];
    std::stringstream msg;
    bool control = true;
    const std::string &requester = client->getNickname();
    std::shared_ptr<Client> targetClient = nullptr;
    std::transform(upperCaseNick.begin(), upperCaseNick.end(),
                   upperCaseNick.begin(), ::toupper);

    for (auto const &it : _nick_to_client) {
        std::string upperCaseIt = it.first;
        std::transform(upperCaseIt.begin(), upperCaseIt.end(),
                       upperCaseIt.begin(), ::toupper);
        if (upperCaseIt == upperCaseNick) {
            targetClient = it.second;
            control = false;
            break;
        }
    }
    if (control) {
        msg.str("");
        msg.clear();
        msg << requester << " " << token.params[0];
        handleMsg(IRCCode::RPL_ENDOFWHOIS, client, "", msg.str());
        return;
    }

    const std::string &targetNickname = targetClient->getNickname();
    const std::string &targetUsername = targetClient->getUsername();
    const std::string &targetIP = targetClient->getIP();
    const std::string &targetRealname = targetClient->getRealname();

    msg << " " << requester << " " << targetNickname << " " << targetUsername
        << " " << targetIP << " * :" << targetRealname;
    handleMsg(IRCCode::RPL_WHOISUSER, client, "", msg.str());

    msg.str("");
    msg.clear();
    msg << " " << targetNickname << " " << serverName << " :ft_irc";
    handleMsg(IRCCode::RPL_WHOISSERVER, client, "", msg.str());

    msg.str("");
    msg.clear();
    msg << requester << " " << targetNickname;
    handleMsg(IRCCode::RPL_ENDOFWHOIS, client, "", msg.str());
}
