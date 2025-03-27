/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   MessageHelper.cpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/03 19:46:47 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/27 21:47:52 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <memory>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"
#include "../include/utils.hpp"

void handleMsg(IRCCode code, const std::shared_ptr<Client> &client,
               const std::string &value, const std::string &msg) {
    std::string errnoAsString = {};

    try {
        errnoAsString = std::to_string(static_cast<std::uint16_t>(code));
        if (errnoAsString.length() < 3) {
            errnoAsString.insert(0, 3 - errnoAsString.length(), '0');
        }
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        return;
    }

    switch (code) {
        case IRCCode::SUCCES:
            break;
        case IRCCode::WELCOME:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " :Welcome to the IRCCodam Network ", client->getFullID()));
            break;
        case IRCCode::YOURHOST:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " ",
                              client->getNickname(), " :Your host is ",
                              serverName, ", running version ", serverVersion));
            break;
        case IRCCode::CREATED:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " :This server was created ", msg));
            break;
        case IRCCode::MYINFO:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " ", serverName, " ", serverVersion, " ", msg));
            break;
        case IRCCode::ISUPPORT:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " CHANMODES=i,t,k,o,l CHANTYPES=# PREFIX=(o)@ STATUSMSG=@ ",
                "NICKLEN=9 NETWORK=", NAME, " PING USERHOST :", msg));
            break;
        case IRCCode::USERHOST:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " ",
                              client->getNickname(), " :", msg));
            break;
        case IRCCode::CHANNELMODEIS:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " ",
                              client->getNickname(), " ", value, " ", msg));
            break;
        case IRCCode::TOPIC:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " ",
                              client->getNickname(), " ", value, " :", msg));
            break;
        case IRCCode::NAMREPLY:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " ",
                              client->getNickname(), " = ", value, " :", msg));
            break;
        case IRCCode::ENDOFNAMES:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                "  ", value, " :End of /NAMES list"));
            break;
        case IRCCode::MOTD:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " :", msg));
            break;
        case IRCCode::MOTDSTART:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " :Message of the Day -"));
            break;
        case IRCCode::ENDOFMOTD:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " :End of /MOTD command"));
            break;
        case IRCCode::NOSUCHNICK:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " ", value, " :No such nick/channel"));
            break;
        case IRCCode::NOSUCHCHANNEL:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " ", value, " :No such channel"));
            break;
        case IRCCode::CANNOTSENDTOCHAN:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, value,
                              " :Cannot send to channel"));
            break;
        case IRCCode::TOMANYCHANNELS:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, value,
                              " :You have joined too many channels"));
            break;
        case IRCCode::NORECIPIENT:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, value,
                              " :No recipient given ", msg));
            break;
        case IRCCode::NOTEXTTOSEND:
            client->appendMessageToQue(formatMessage(":", serverName, " ",
                                                     errnoAsString, value,
                                                     " :No text to send"));
            break;
        case IRCCode::UNKNOWNCOMMAND:
            client->appendMessageToQue(formatMessage(":", serverName, " ",
                                                     errnoAsString, value,
                                                     " :Unknow command"));
            break;
        case IRCCode::FILEERROR:
            std::cerr << "Need to impl this" << '\n';
            break;
        case IRCCode::NONICK:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " * ", value,
                              " :No nickname given"));
            break;
        case IRCCode::ERRONUENICK:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " * ", value,
                              " :Erronues nickname"));
            break;
        case IRCCode::NICKINUSE: {
            std::string clientNick = " * ";
            if (client->getNickname() != "") {
                clientNick = " " + client->getNickname() + " ";
            }
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, clientNick,
                              value, " :Nickname is already in use"));
            break;
        }
        case IRCCode::USERNOTINCHANNEL:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " ", value, " ", msg, " :They aren't on that channel"));
            break;
        case IRCCode::NOTOCHANNEL:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, value,
                              " :You're not on that channel"));
            break;
        case IRCCode::USERONCHANNEL:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " ", value,
                              " ", msg, " :is already in channel"));
            break;
        case IRCCode::NOTREGISTERED:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString,
                              " :You have not registered"));
            break;
        case IRCCode::NEEDMOREPARAMS:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, value,
                              " :Not enough parameters"));
            break;
        case IRCCode::ALREADYREGISTERED:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString,
                              " :You may not reregister"));
            break;
        case IRCCode::PASSWDMISMATCH:
            client->appendMessageToQue(formatMessage(":", serverName, " ",
                                                     errnoAsString, " *",
                                                     " :Password incorrect"));
            break;
        case IRCCode::KEYSET: // checken if channel is 1 index of the params
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, value,
                              " :Channel key already set"));
            break;
        case IRCCode::CHANNELISFULL:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " ", value,
                              " :Cannot join channel (+l)"));
            break;
        case IRCCode::UNKNOWMODE: // checken if char is 2 index of the params
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, value,
                              " :is unknown mode char to me"));
            break;
        case IRCCode::INVITEONLYCHAN:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString, " ", value,
                              " :Cannot join channel (+i)"));
            break;
        case IRCCode::BADCHANNELKEY:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                +" " + value, " :Cannot join channel (+k) - bad key"));
            break;
        case IRCCode::NOPRIVILEGES:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString,
                " :Permission Denied- You're not an IRC operator"));
            break;
        case IRCCode::CHANOPRIVSNEEDED:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " ", value, " :You're not channel operator"));
            break;
        case IRCCode::UMODEUNKNOWNFLAG:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " :Unknown MODE flag"));
            break;
        case IRCCode::USERSDONTMATCH:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", errnoAsString,
                              " :Cant change mode for other users"));
            break;
        case IRCCode::INVALIDMODEPARAM:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", errnoAsString, " ", client->getNickname(),
                " ", value, " :Invalid value: '", msg, "'"));
            break;
        case IRCCode::PART:
            client->appendMessageToQue(formatMessage(
                ":", value, " PART ", msg));
            break;
        case IRCCode::JOIN:
            client->appendMessageToQue(formatMessage(
                ":", value, " JOIN ", msg));
            break;
        case IRCCode::NICKCHANGED:
            client->appendMessageToQue(
                formatMessage(":", value, " NICK ", msg));
            break;
        case IRCCode::PRIVMSG:
            client->appendMessageToQue(formatMessage(
                ":", value, " PRIVMSG ", client->getNickname(), msg));
            break;
        default:
            std::cerr << "Unknow IRCCode: " << errnoAsString << '\n';
    }
}
