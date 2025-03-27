/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Enums.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/27 21:58:48 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/27 16:44:26 by lvan-gef      ########   odam.nl         */
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
    USERLIMIT = std::numeric_limits<uint16_t>::max()
};
bool operator>(std::uint16_t lhs, Defaults rhs);
bool operator<(std::uint16_t lhs, Defaults rhs);

enum class ChannelMode : std::uint8_t {
    INVITE_ONLY = 1 << 0,
    TOPIC_PROTECTED = 1 << 1,
    PASSWORD_PROTECTED = 1 << 2,
    OPERATOR = 1 << 3,
    USER_LIMIT = 1 << 4,
};

/**
 * @brief Bitwise OR assignment operator for combining flags.
 *
 * @param lhs Reference to the target bitmask (std::uint8_t)
 * @param rhs ChannelMode flag to enable      (ChannelMode)
 * @return Reference to modified lhs          (std::uint_t)
 */
std::uint8_t &operator|=(std::uint8_t &lhs, ChannelMode rhs);

/**
 * @brief Bitwise AND assignment operator for clearing flags.
 * @param lhs Reference to the target bitmask (std::uint8_t)
 * @param rhs ChannelMode flag to disable     (ChannelMode)
 * @return Reference to modified lhs          (std::uint8_t)
 */
std::uint8_t &operator&=(std::uint8_t &lhs, ChannelMode rhs);

/**
 * @brief Bitwise NOT operator for flag inversion.
 * @param rhs ChannelMode flag to invert      (ChannelMode)
 * @return std::uint8_t with all bits flipped (std::uint_8_t)
 */
std::uint8_t operator~(ChannelMode rhs);

/**
 * @brief Bitwise OR operator for combining flags.
 * @param lhs First ChannelMode flag     (ChannelMode)
 * @param rhs Second ChannelMode flag    (ChannelMode)
 * @return std::uint8_t combined bitmask (std::uint8_t)
 */
std::uint8_t operator|(ChannelMode lhs, ChannelMode rhs);

/**
 * @brief Bitwise AND operator for checking flags.
 * @param lhs Bitmask to check             (std::uint8_t)
 * @param rhs ChannelMode flag to test     (ChannelMode)
 * @return std::uint8_t with matching bits
 */
std::uint8_t operator&(std::uint8_t lhs, ChannelMode rhs);

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
    SUCCES = 0, // For my own use
    USERHOST = 302,
    CHANNELMODEIS = 324,
    TOPIC = 332,
    NAMREPLY = 353,
    ENDOFNAMES = 366,
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
    INVALIDMODEPARAM = 696 // Not in IRC but in unrealircd
};

#endif // ENUMS_HPP
