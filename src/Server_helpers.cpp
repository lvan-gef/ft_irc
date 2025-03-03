/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server_helpers.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/03 19:46:47 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/03 21:17:00 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstdarg>
#include <cstdint>
#include <initializer_list>
#include <sstream>
#include <string>
#include <vector>

#include "../include/Enums.hpp"
#include "../include/Server.hpp"

void Server::_handleError(std::vector<std::string> params, IRCCodes code,
                          int clientFD) {
    switch (code) {
        case IRCCodes::NOSUCHNICK:
            break;
        case IRCCodes::NOSUCHCHANNEL:
            break;
        case IRCCodes::CANNOTSENDTOCHAN:
            break;
        case IRCCodes::TOMANYCHANNELS:
            break;
        case IRCCodes::WASNOSUCHNICK:
            break;
        case IRCCodes::TOOMANYTARGETS:
            break;
        case IRCCodes::NOSUCHSERVICE:
            break;
        case IRCCodes::NOORIGIN:
            break;
        case IRCCodes::NORECIPIENT:
            break;
        case IRCCodes::NOTEXTTOSEND:
            break;
        case IRCCodes::NOTOPLEVEL:
            break;
        case IRCCodes::WILDTOPLEVEL:
            break;
        case IRCCodes::UNKNOWCOMMAND:
            break;
        case IRCCodes::NOADMININFO:
            break;
        case IRCCodes::FILEERROR:
            break;
        case IRCCodes::NONICK: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(
                                   IRCCodes::ERRONUENICK)),
                               " * ", params[0], " :No nickname given");
            _sendMessage(clientFD, fstring);
            break;
        }
        case IRCCodes::ERRONUENICK: {
            std::string fstring =
                _formatMessage(std::to_string(static_cast<std::uint16_t>(
                                   IRCCodes::ERRONUENICK)),
                               " * ", params[0], " :Erronues nickname");
            _sendMessage(clientFD, fstring);
            break;
        }
        case IRCCodes::NICKINUSE: {
            std::string fstring = _formatMessage(
                std::to_string(static_cast<std::uint16_t>(IRCCodes::NICKINUSE)),
                " * ", params[0], " :Nickname is already in use");
            _sendMessage(clientFD, fstring);
            break;
        }
        case IRCCodes::NICKCOLLISION:
            break;
        case IRCCodes::UNAVAILRESOURCE:
            break;
        case IRCCodes::USERNOTINCHANNEL:
            break;
        case IRCCodes::NOTOCHANNEL:
            break;
        case IRCCodes::USERONCHANNEL:
            break;
        case IRCCodes::NOTREGISTERED:
            break;
        case IRCCodes::NEEDMOREPARAMS:
            break;
        case IRCCodes::ALREADYREGISTERED:
            break;
        case IRCCodes::NOPERMFORHOST:
            break;
        case IRCCodes::PASSWDMISMATCH:
            break;
        case IRCCodes::YOURBANNEDCREEP:
            break;
        case IRCCodes::CHANNELISFULL:
            break;
        case IRCCodes::UNKNOWMODE:
            break;
        case IRCCodes::INVITEONLYCHAN:
            break;
        case IRCCodes::BANNEDFROMCHAN:
            break;
        case IRCCodes::BADCHANNELKEY:
            break;
        case IRCCodes::BADCHANMASK:
            break;
        case IRCCodes::CHANOPRIVSNEEDED:
            break;
        case IRCCodes::CANTKILLSERVER:
            break;
        case IRCCodes::NOOPERHOST:
            break;
        case IRCCodes::UMODEUNKNOWNFLAG:
            break;
        case IRCCodes::USERSDONTMATCH:
            break;
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
