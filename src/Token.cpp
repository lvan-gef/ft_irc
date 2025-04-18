/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Token.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/27 14:59:36 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/04/02 16:52:22 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "../include/Enums.hpp"
#include "../include/Token.hpp"

std::vector<IRCMessage> parseIRCMessage(const std::string &msg) {
    std::vector<IRCMessage> tokens;
    std::string word;
    std::vector<std::string> lines = split(msg, "\r\n");

    for (const std::string &line : lines) {
        IRCMessage parsed;
        std::istringstream stream(line);
        if (line[0] == ':') {
            stream >> parsed.prefix;
            parsed.prefix.erase(0, 1);
        }

        if (stream >> parsed.command) {
            while (stream >> word) {
                if (word[0] == ':') {
                    std::string rest;
                    std::getline(stream, rest);
                    parsed.params.emplace_back(word.substr(1) + rest);
                    break;
                } else {
                    parsed.params.emplace_back(word);
                }
            }
        }

        parsed.success = true;
        /*parsed.err = IRCCodes::SUCCES;*/
        parsed.type = getCommand(parsed.command);
        std::cerr << "type: " << parsed.command << '\n';
        if (parsed.command != "") {
            tokens.emplace_back(parsed);
        }
    }

    return tokens;
}

IRCCommand getCommand(const std::string &command) {
    static const std::unordered_map<std::string, IRCCommand> commandMap = {
        {"NICK", IRCCommand::NICK},         {"USER", IRCCommand::USER},
        {"PASS", IRCCommand::PASS},         {"PRIVMSG", IRCCommand::PRIVMSG},
        {"JOIN", IRCCommand::JOIN},         {"TOPIC", IRCCommand::TOPIC},
        {"PART", IRCCommand::PART},         {"QUIT", IRCCommand::QUIT},
        {"PING", IRCCommand::PING},         {"KICK", IRCCommand::KICK},
        {"INVITE", IRCCommand::INVITE},     {"MODE", IRCCommand::MODE},
        {"USERHOST", IRCCommand::USERHOST}, {"UNKNOW", IRCCommand::UNKNOW}};

    auto it = commandMap.find(command);
    if (it == commandMap.end()) {
        return IRCCommand::UNKNOW;
    }

    return it->second;
}

// debugging
void IRCMessage::print() const {
    std::cout << "prefix: '" << prefix << "'" << '\n';
    std::cout << "command: '" << command << "'" << '\n';
    std::cout << "param: ";
    for (const std::string &param : params) {
        std::cout << param << ", ";
    }
    std::cout << '\n';
}

void IRCMessage::setIRCCode(const IRCCode &code) noexcept {
    err.set_value(code);
}
