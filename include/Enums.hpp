/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Enums.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/27 21:58:48 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/28 14:26:39 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <cstdint>
#include <limits>

#include <sys/socket.h>

enum class Defaults : std::uint16_t {
    LOWEST_PORT = 1024,
    HIGHEST_PORT = 65535,
    EVENT_SIZE = 2048,
    MAX_CONNECTIONS = SOMAXCONN,
    READ_SIZE = 512,
    INTERVAL = 1000,
    TIMEOUT = 360,
    USERLIMIT = std::numeric_limits<uint16_t>::max(),
    NICKLEN = 9,
};
bool operator>(std::uint16_t lhs, Defaults rhs);
bool operator<(std::uint16_t lhs, Defaults rhs);
std::uint16_t getDefaultValue(Defaults rhs);

enum class ChannelMode : std::uint8_t {
    INVITE_ONLY,
    TOPIC_PROTECTED,
    PASSWORD_PROTECTED,
    OPERATOR,
    USER_LIMIT,
};

enum class ChannelCommand : std::uint8_t {
    MODE_I = 'i',
    MODE_T = 't',
    MODE_K = 'k',
    MODE_O = 'o',
    MODE_L = 'l'
};

enum class IRCCommand : std::uint8_t {
    NICK,
    USER,
    PASS,
    PRIVMSG,
    JOIN,
    TOPIC,
    PART,
    QUIT,
    PING,
    KICK,
    INVITE,
    MODE,
    USERHOST,
    UNKNOW
};

enum class IRCCode : std::int16_t {
    WELCOME = 1,
    YOURHOST = 2,
    CREATED = 3,
    MYINFO = 4,
    ISUPPORT = 5,
    USERHOST = 302,
    CHANNELMODEIS = 324,
    TOPIC = 332,
    NAMREPLY = 353,
    ENDOFNAMES = 366,
    MOTD = 372,
    MOTDSTART = 375,
    ENDOFMOTD = 376,
    NOSUCHNICK = 401,
    NOSUCHCHANNEL = 403,
    CANNOTSENDTOCHAN = 404,
    TOMANYCHANNELS = 405,
    /*WASNOSUCHNICK = 406,  // think we dont need to impl this*/
    /*TOOMANYTARGETS = 407,  // think we dont need to impl this*/
    /*NOSUCHSERVICE = 408, // not sure if we need to impl it*/
    /*NOORIGIN = 409,      // not sure if we need to impl it*/
    NORECIPIENT = 411,
    NOTEXTTOSEND = 412,
    /*NOTOPLEVEL = 413,  // not sure if we need to impl it*/
    /*WILDTOPLEVEL = 414, // not sure if we need to impl it*/
    UNKNOWNCOMMAND = 421,
    /*NOADMININFO = 423,  // think we dont need to impl this*/
    FILEERROR = 424, // only for bonus we need to impl thid
    NONICK = 431,
    ERRONUENICK = 432,
    NICKINUSE = 433,
    /*NICKCOLLISION = 436,  // think we dont need to impl this*/
    /*UNAVAILRESOURCE = 437,  // cannot find it in the rfc*/
    USERNOTINCHANNEL = 441,
    NOTOCHANNEL = 442,
    USERONCHANNEL = 443,
    /*USERSDISABLED = 446,  // not sure if we need to impl it*/
    NOTREGISTERED = 451,
    NEEDMOREPARAMS = 461,
    ALREADYREGISTERED = 462,
    /*NOPERMFORHOST = 463,*/
    PASSWDMISMATCH = 464,
    /*YOURBANNEDCREEP = 465,  // for channel or for server ban??*/
    KEYSET = 467,
    CHANNELISFULL = 471,
    UNKNOWMODE = 472,
    INVITEONLYCHAN = 473,
    BADCHANNELKEY = 475,
    /*BADCHANMASK = 476,  // can not found it in the rfc*/
    NOPRIVILEGES = 481,
    CHANOPRIVSNEEDED = 482,
    /*CANTKILLSERVER = 483,  // not sure if we need to impl it*/
    UMODEUNKNOWNFLAG = 501,
    USERSDONTMATCH = 502,
    INVALIDMODEPARAM = 696, // Not in IRC but in unrealircd
    TOPICNOTICE = 994,      // for my own use
    KICK = 995,             // for my own use
    PART = 996,             // for my own use
    JOIN = 997,             // for my own use
    NICKCHANGED = 998,      // for my own use
    PRIVMSG = 999,          // for my own use
};

#endif // ENUMS_HPP
