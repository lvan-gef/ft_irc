/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ServerErrorHelper.cpp                              :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/03 19:46:47 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/07 19:35:17 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstdarg>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"

void Server::_handleError(IRCMessage message,
                          const std::shared_ptr<Client> &client) {
    int clientFD = client->getFD();
    std::string errnoAsString;
    IRCCodes error = {};

    try {
        errnoAsString =
            std::to_string(static_cast<std::uint16_t>(message.err.get_value()));
        error = message.err.get_value();
    } catch (std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        return;
    }

    switch (error) {
        case IRCCodes::NOSUCHNICK:
            _sendMessage(clientFD, errnoAsString, message.params[0],
                         " :No such nick/channel");
            break;
        case IRCCodes::NOSUCHCHANNEL:
            _sendMessage(clientFD, errnoAsString, message.params[0],
                         " :No such channel");
            break;
        case IRCCodes::CANNOTSENDTOCHAN:
            _sendMessage(clientFD, errnoAsString, message.params[0],
                         " :Cannot send to channel");
            break;
        case IRCCodes::TOMANYCHANNELS:
            _sendMessage(clientFD, errnoAsString, message.params[0],
                         " :You have joined too many channels");
            break;
        case IRCCodes::NORECIPIENT:
            _sendMessage(clientFD, errnoAsString, message.params[0],
                         " :No recipient given ", message.command);
            break;
        case IRCCodes::NOTEXTTOSEND:
            _sendMessage(clientFD, errnoAsString, message.params[0],
                         " :No text to send");
            break;
        case IRCCodes::UNKNOWCOMMAND:
            _sendMessage(clientFD, errnoAsString, message.command,
                         " :Unknow command");
            break;
        case IRCCodes::FILEERROR:
            std::cerr << "Need to impl this" << '\n';
            break;
        case IRCCodes::NONICK:
            _sendMessage(clientFD, errnoAsString, " * ", message.params[0],
                         " :No nickname given");
            break;
        case IRCCodes::ERRONUENICK:
            _sendMessage(clientFD, errnoAsString, " * ", message.params[0],
                         " :Erronues nickname");
            break;
        case IRCCodes::NICKINUSE: {
            std::string clientNick = " * ";
            if (client->getNickname() != "") {
                clientNick = " " + client->getNickname() + " ";
            }
            _sendMessage(clientFD, errnoAsString, clientNick, message.params[0],
                         " :Nickname is already in use");
            break;
        }
        case IRCCodes::USERNOTINCHANNEL:
            _sendMessage(clientFD, errnoAsString, message.params[0], " ",
                         message.params[1], " :They aren't on that channel");
            break;
        case IRCCodes::NOTOCHANNEL:
            _sendMessage(clientFD, errnoAsString, message.params[0],
                         " :You're not on that channel");
            break;
        case IRCCodes::USERONCHANNEL:
            _sendMessage(clientFD, errnoAsString, message.params[0], " ",
                         message.params[1], " :is already in channel");
            break;
        case IRCCodes::NOTREGISTERED:
            _sendMessage(clientFD, errnoAsString, " :You have not registered");
            break;
        case IRCCodes::NEEDMOREPARAMS:
            _sendMessage(clientFD, errnoAsString, message.command,
                         " :Not enough parameters");
            break;
        case IRCCodes::ALREADYREGISTERED:
            _sendMessage(clientFD, errnoAsString, " :You may not reregister");
            break;
        case IRCCodes::PASSWDMISMATCH:
            _sendMessage(clientFD, errnoAsString, " :Password incorrect");
            break;
        case IRCCodes::KEYSET: // checken if channel is 1 index of the params
            _sendMessage(clientFD, errnoAsString, message.params[1],
                         " :Channel key already set");
            break;
        case IRCCodes::CHANNELISFULL: // checken if channel is 1 index of the
                                      // params
            _sendMessage(clientFD, errnoAsString, message.params[1],
                         " :Cannot join channel (+l)");
            break;
        case IRCCodes::UNKNOWMODE: // checken if char is 2 index of the params
            _sendMessage(clientFD, errnoAsString, message.params[2],
                         " :is unknown mode char to me");
            break;
        case IRCCodes::INVITEONLYCHAN:
            _sendMessage(clientFD, errnoAsString, message.params[1],
                         " :Cannot join channel (+i)");
            break;
        case IRCCodes::BANNEDFROMCHAN:
            _sendMessage(clientFD, errnoAsString, message.params[1],
                         " :Cannot join channel (+b)");
            break;
        case IRCCodes::BADCHANNELKEY:
            _sendMessage(clientFD, errnoAsString, message.params[1],
                         " :Cannot join channel (+k)");
            break;
        case IRCCodes::NOPRIVILEGES:
            _sendMessage(clientFD, errnoAsString,
                         " :Permission Denied- You're not an IRC operator");
            break;
        case IRCCodes::CHANOPRIVSNEEDED:
            _sendMessage(clientFD, errnoAsString, message.params[1],
                         " :You're not channel operator");
            break;
        case IRCCodes::UMODEUNKNOWNFLAG:
            _sendMessage(clientFD, errnoAsString, " :Unknown MODE flag");
            break;
        case IRCCodes::USERSDONTMATCH:
            _sendMessage(clientFD, errnoAsString,
                         " :Cant change mode for other users");
            break;
    }
}
