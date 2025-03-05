/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server_helpers.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/03 19:46:47 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/04 20:14:22 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstdarg>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"

void Server::_handleError(IRCMessage message, const std::shared_ptr<Client> &client) {
    switch (message.err) {
        case IRCCodes::NOSUCHNICK: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[0], " :No such nick/channel");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::NOSUCHCHANNEL: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[0], " :No such channel");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::CANNOTSENDTOCHAN: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[0], " :Cannot send to channel");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::TOMANYCHANNELS: {
            std::string fstring = _formatMessage(
                std::to_string(static_cast<std::uint16_t>(message.err)),
                message.params[0], " :You have joined too many channels");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::NORECIPIENT: {
            std::string fstring = _formatMessage(
                std::to_string(static_cast<std::uint16_t>(message.err)),
                message.params[0], " :No recipient given ", message.command);
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::NOTEXTTOSEND: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[0], " :No text to send");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::UNKNOWCOMMAND: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.command, " :Unknow command");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::FILEERROR:
            std::cerr << "Need to impl this" << '\n';
            break;
        case IRCCodes::NONICK: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               " * ", message.params[0], " :No nickname given");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::ERRONUENICK: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               " * ", message.params[0], " :Erronues nickname");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::NICKINUSE: {
            std::string clientNick = " * ";
            if (client->getNickname() != "") {
                clientNick = " " + client->getNickname() + " ";
            }
            std::string fstring = _formatMessage(
                std::to_string(static_cast<std::uint16_t>(message.err)), clientNick,
                message.params[0], " :Nickname is already in use");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::USERNOTINCHANNEL: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[0], " ", message.params[1],
                               " :They aren't on that channel");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::NOTOCHANNEL: {
            std::string fstring = _formatMessage(
                std::to_string(static_cast<std::uint16_t>(message.err)),
                message.params[0], " :You're not on that channel");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::USERONCHANNEL: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[0], " ", message.params[1],
                               " :is already in channel");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::NOTREGISTERED: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               " :You have not registered");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::NEEDMOREPARAMS: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.command, " :Not enough parameters");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::ALREADYREGISTERED: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               " :You may not reregister");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::PASSWDMISMATCH: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               " :Password incorrect");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::KEYSET: { // checken if channel is 1 index of the params
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[1], " :Channel key already set");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::CHANNELISFULL: { // checken if channel is 1 index of the
                                        // params
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[1], " :Cannot join channel (+l)");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::UNKNOWMODE: { // checken if char is 2 index of the params
            std::string fstring = _formatMessage(
                std::to_string(static_cast<std::uint16_t>(message.err)),
                message.params[2], " :is unknown mode char to me");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::INVITEONLYCHAN: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[1], " :Cannot join channel (+i)");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::BANNEDFROMCHAN: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[1], " :Cannot join channel (+b)");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::BADCHANNELKEY: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               message.params[1], " :Cannot join channel (+k)");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::NOPRIVILEGES: {
            std::string fstring = _formatMessage(
                std::to_string(static_cast<std::uint16_t>(message.err)),
                " :Permission Denied- You're not an IRC operator");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::CHANOPRIVSNEEDED: {
            std::string fstring = _formatMessage(
                std::to_string(static_cast<std::uint16_t>(message.err)),
                message.params[1], " :You're not channel operator");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::UMODEUNKNOWNFLAG: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               " :Unknown MODE flag");
            _sendMessage(client->getFD(), fstring);
            break;
        }
        case IRCCodes::USERSDONTMATCH: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(message.err)),
                               " :Cant change mode for other users");
            _sendMessage(client->getFD(), fstring);
            break;
        }
    }
}

template <typename... Args>
std::string Server::_formatMessage(const Args &...args) noexcept {
    std::ostringstream oss;

    oss << ":" << _serverName << " ";
    (void)std::initializer_list<int>{(oss << args, 0)...};
    oss << "\r\n";
    return oss.str();
}
