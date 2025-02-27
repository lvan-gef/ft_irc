/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Enums.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/27 21:58:48 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/02/27 21:58:48 by lvan-gef      ########   odam.nl         */
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
    UNKNOW
};

enum class IRCCodes : std::int16_t {
    NOSUCHNICK = 401,
    NOSUCHCHANNEL = 403,
    CANNOTSENDTOCHAN = 404,
    TOMANYCHANNELS = 405,
    WASNOSUCHNICK = 406,
    NOORIGIN = 409,  // not sure if we need to impl it
    NONICK = 431,
    ERRONUENICK = 432,
    NICKINUSE = 433,
};

#endif // ENUMS_HPP
