/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Enums.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/27 21:58:48 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/04 18:21:08 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <cstdint>

enum class IRCCommand : std::uint8_t {
    NICK,
    USER,
    PASS,
    PRIVMSG,
    JOIN,
    PART,
    QUIT,
    PING,
    KICK,
    INVITE,
    MODE_I,
    MODE_T,
    MODE_K,
    MODE_O,
    MODE_L,
    UNKNOW
};

enum class IRCCodes : std::int16_t {
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
    UNKNOWCOMMAND = 421,
    /*NOADMININFO = 423,  // think we dont need to impl this*/
    FILEERROR = 424,  // only for bonus we need to impl thid
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
    BANNEDFROMCHAN = 474,
    BADCHANNELKEY = 475,
    /*BADCHANMASK = 476,  // can not found it in the rfc*/
    NOPRIVILEGES = 481,
    CHANOPRIVSNEEDED = 482,
    /*CANTKILLSERVER = 483,  // not sure if we need to impl it*/
    UMODEUNKNOWNFLAG = 501,
    USERSDONTMATCH = 502,
};

#endif // ENUMS_HPP
