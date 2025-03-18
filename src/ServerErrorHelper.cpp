/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerErrorHelper.cpp                              :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/03 19:46:47 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/17 21:03:05 by lvan-gef      ########   odam.nl         */
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
    IRCCodes error = {};

    try {
        error = token.err.get_value();
        errnoAsString = std::to_string(static_cast<std::uint16_t>(error));
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        return;
    }

    switch (error) {
        case IRCCodes::SUCCES:
            break;
        case IRCCodes::NOSUCHNICK:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[0], " :No such nick/channel"));
            break;
        case IRCCodes::NOSUCHCHANNEL:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " ",
                              client->getNickname(), " ", token.params[0],
                              " :No such channel"));
            break;
        case IRCCodes::CANNOTSENDTOCHAN:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[0], " :Cannot send to channel"));
            break;
        case IRCCodes::TOMANYCHANNELS:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, token.params[0],
                " :You have joined too many channels"));
            break;
        case IRCCodes::NORECIPIENT:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, token.params[0],
                " :No recipient given ", token.command));
            break;
        case IRCCodes::NOTEXTTOSEND:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[0], " :No text to send"));
            break;
        case IRCCodes::UNKNOWCOMMAND:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.command, " :Unknow command"));
            break;
        case IRCCodes::FILEERROR:
            std::cerr << "Need to impl this" << '\n';
            break;
        case IRCCodes::NONICK:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " * ",
                              token.params[0], " :No nickname given"));
            break;
        case IRCCodes::ERRONUENICK:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " * ",
                              token.params[0], " :Erronues nickname"));
            break;
        case IRCCodes::NICKINUSE: {
            std::string clientNick = " * ";
            if (client->getNickname() != "") {
                clientNick = " " + client->getNickname() + " ";
            }
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, clientNick,
                              token.params[0], " :Nickname is already in use"));
            break;
        }
        case IRCCodes::USERNOTINCHANNEL:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, token.params[0], " ",
                token.params[1], " :They aren't on that channel"));
            break;
        case IRCCodes::NOTOCHANNEL:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[0], " :You're not on that channel"));
            break;
        case IRCCodes::USERONCHANNEL:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, token.params[0], " ",
                token.params[1], " :is already in channel"));
            break;
        case IRCCodes::NOTREGISTERED:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              " :You have not registered"));
            break;
        case IRCCodes::NEEDMOREPARAMS:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.command, " :Not enough parameters"));
            break;
        case IRCCodes::ALREADYREGISTERED:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              " :You may not reregister"));
            break;
        case IRCCodes::PASSWDMISMATCH:
            client->appendMessageToQue(formatMessage(":", _serverName, " ",
                                                     errnoAsString, " *",
                                                     " :Password incorrect"));
            break;
        case IRCCodes::KEYSET: // checken if channel is 1 index of the params
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[1], " :Channel key already set"));
            break;
        case IRCCodes::CHANNELISFULL: // checken if channel is 1 index of the
                                      // params
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[1], " :Cannot join channel (+l)"));
            break;
        case IRCCodes::UNKNOWMODE: // checken if char is 2 index of the params
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[2], " :is unknown mode char to me"));
            break;
        case IRCCodes::INVITEONLYCHAN:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[1], " :Cannot join channel (+i)"));
            break;
        case IRCCodes::BANNEDFROMCHAN:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              token.params[1], " :Cannot join channel (+b)"));
            break;
        case IRCCodes::BADCHANNELKEY:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString, " ",
                              client->getNickname(), token.params[1],
                              " :Cannot join channel (+k) - bad key"));
            break;
        case IRCCodes::NOPRIVILEGES:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString,
                " :Permission Denied- You're not an IRC operator"));
            break;
        case IRCCodes::CHANOPRIVSNEEDED:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, token.params[1],
                " :You're not channel operator"));
            break;
        case IRCCodes::UMODEUNKNOWNFLAG:
            client->appendMessageToQue(formatMessage(
                ":", _serverName, " ", errnoAsString, " :Unknown MODE flag"));
            break;
        case IRCCodes::USERSDONTMATCH:
            client->appendMessageToQue(
                formatMessage(":", _serverName, " ", errnoAsString,
                              " :Cant change mode for other users"));
            break;
    }
}
