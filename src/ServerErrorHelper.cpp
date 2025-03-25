/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerErrorHelper.cpp                              :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/03 19:46:47 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/25 20:59:56 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <memory>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/utils.hpp"

void Server::_handleError(IRCMessage token,
                          const std::shared_ptr<Client> &client) {
    std::string errnoAsString = {};
    IRCCode error = {};

    try {
        error = token.err.get_value();
        errnoAsString = std::to_string(static_cast<std::uint16_t>(error));
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        return;
    }

    switch (error) {
        case IRCCode::SUCCES:
            break;
        case IRCCode::NOSUCHNICK:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " ",
                              client->getNickname(), " ", token.params[0],
                              " :No such nick/channel"));
            break;
        case IRCCode::NOSUCHCHANNEL:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " ",
                              client->getNickname(), " ", token.params[0],
                              " :No such channel"));
            break;
        case IRCCode::CANNOTSENDTOCHAN:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[0], " :Cannot send to channel"));
            break;
        case IRCCode::TOMANYCHANNELS:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, token.params[0],
                " :You have joined too many channels"));
            break;
        case IRCCode::NORECIPIENT:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, token.params[0],
                " :No recipient given ", token.command));
            break;
        case IRCCode::NOTEXTTOSEND:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[0], " :No text to send"));
            break;
        case IRCCode::UNKNOWNCOMMAND:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.command, " :Unknow command"));
            break;
        case IRCCode::FILEERROR:
            std::cerr << "Need to impl this" << '\n';
            break;
        case IRCCode::NONICK:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " * ",
                              token.params[0], " :No nickname given"));
            break;
        case IRCCode::ERRONUENICK:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " * ",
                              token.params[0], " :Erronues nickname"));
            break;
        case IRCCode::NICKINUSE: {
            std::string clientNick = " * ";
            if (client->getNickname() != "") {
                clientNick = " " + client->getNickname() + " ";
            }
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, clientNick,
                              token.params[0], " :Nickname is already in use"));
            break;
        }
        case IRCCode::USERNOTINCHANNEL:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, " ",
                client->getNickname(), " ", token.params[0], " ",
                token.params[1], " :They aren't on that channel"));
            break;
        case IRCCode::NOTOCHANNEL:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[0], " :You're not on that channel"));
            break;
        case IRCCode::USERONCHANNEL:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, token.params[0], " ",
                token.params[1], " :is already in channel"));
            break;
        case IRCCode::NOTREGISTERED:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              " :You have not registered"));
            break;
        case IRCCode::NEEDMOREPARAMS:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.command, " :Not enough parameters"));
            break;
        case IRCCode::ALREADYREGISTERED:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              " :You may not reregister"));
            break;
        case IRCCode::PASSWDMISMATCH:
            client->appendMessageToQue(formatMessage(":", _serverName, " ",
                                                     errnoAsString, " *",
                                                     " :Password incorrect"));
            break;
        case IRCCode::KEYSET: // checken if channel is 1 index of the params
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[1], " :Channel key already set"));
            break;
        case IRCCode::CHANNELISFULL: // checken if channel is 1 index of the
                                     // params
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[1], " :Cannot join channel (+l)"));
            break;
        case IRCCode::UNKNOWMODE: // checken if char is 2 index of the params
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[2], " :is unknown mode char to me"));
            break;
        case IRCCode::INVITEONLYCHAN:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[1], " :Cannot join channel (+i)"));
            break;
        case IRCCode::BADCHANNELKEY:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " ",
                              client->getNickname(), token.params[1],
                              " :Cannot join channel (+k) - bad key"));
            break;
        case IRCCode::NOPRIVILEGES:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString,
                " :Permission Denied- You're not an IRC operator"));
            break;
        case IRCCode::CHANOPRIVSNEEDED:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " ",
                              client->getNickname(), " ", token.params[0],
                              " :You're not channel operator"));
            break;
        case IRCCode::UMODEUNKNOWNFLAG:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, " :Unknown MODE flag"));
            break;
        case IRCCode::USERSDONTMATCH:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              " :Cant change mode for other users"));
            break;
    }
}
