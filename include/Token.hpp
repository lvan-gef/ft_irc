/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Token.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/27 14:59:50 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/02/27 20:17:58 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <sys/types.h>
#include <vector>

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

struct IRCMessage {
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    ssize_t err;
    IRCCommand type;

    void print() const;
};

std::vector<IRCMessage> parseIRCMessage(const std::string &msg);
IRCCommand getCommand(const std::string &command);
std::vector<std::string> split(const std::string &str,
                               const std::string &delim);

#endif // !TOKEN_HPP
