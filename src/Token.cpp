/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Token.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/27 14:59:36 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/04/22 20:39:38 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "../include/Enums.hpp"
#include "../include/Token.hpp"

static IRCCommand getCommand(const std::string &command);
static void validateMessage(std::vector<IRCMessage> &tokens);
static void isValidNick(IRCMessage &msg);
static void isValidUsername(IRCMessage &msg);

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

        parsed.succes = true;
        parsed.type = getCommand(parsed.command);
        parsed.errMsg = "";
        if (parsed.command != "") {
            tokens.emplace_back(parsed);
        }
    }

    validateMessage(tokens);

    return tokens;
}

static IRCCommand getCommand(const std::string &command) {
    static const std::unordered_map<std::string, IRCCommand> commandMap = {
        {"CAP", IRCCommand::CAP},         {"NICK", IRCCommand::NICK},
        {"USER", IRCCommand::USER},       {"PASS", IRCCommand::PASS},
        {"PRIVMSG", IRCCommand::PRIVMSG}, {"JOIN", IRCCommand::JOIN},
        {"TOPIC", IRCCommand::TOPIC},     {"PART", IRCCommand::PART},
        {"QUIT", IRCCommand::QUIT},       {"PING", IRCCommand::PING},
        {"KICK", IRCCommand::KICK},       {"INVITE", IRCCommand::INVITE},
        {"MODE", IRCCommand::MODE},       {"USERHOST", IRCCommand::USERHOST},
        {"UNKNOW", IRCCommand::UNKNOW}};

    auto it = commandMap.find(command);
    if (it == commandMap.end()) {
        return IRCCommand::UNKNOW;
    }

    return it->second;
}

static void validateMessage(std::vector<IRCMessage> &tokens) {
    for (auto &&token : tokens) {
        switch (token.type) {
            case IRCCommand::CAP:
                break;
            case IRCCommand::NICK:
                isValidNick(token);
                break;
            case IRCCommand::USER:
                isValidUsername(token);
                break;
            case IRCCommand::PASS:
                if (token.params.size() < 1) {
                    token.err.set_value(IRCCode::NEEDMOREPARAMS);
                    token.succes = false;
                }
                break;
            case IRCCommand::PRIVMSG:
                if (token.params.size() < 2) {
                    token.err.set_value(IRCCode::NEEDMOREPARAMS);
                    token.succes = false;
                }
                break;
            case IRCCommand::JOIN:
                if (token.params.size() < 1) {
                    token.err.set_value(IRCCode::NEEDMOREPARAMS);
                    token.succes = false;
                }

                if (token.params[0][0] != '#') {
                    token.err.set_value(IRCCode::NOSUCHCHANNEL);
                    token.succes = false;
                }
                break;
            case IRCCommand::TOPIC:
                if (token.params.size() > 1 && token.params[0].length() > 0) {
                    for (char c : token.params[0]) {
                        if (c < 32 || c == 127) {
                            token.err.set_value(IRCCode::TOPIC);
                            token.succes = false;
                            token.errMsg = "Invalid topic";
                        }
                    }
                }

                if (token.params.size() > getDefaultValue(Defaults::TOPICLEN)) {
                    token.err.set_value(IRCCode::TOPIC);
                    token.succes = false;
                    token.errMsg = "Topic to long";
                }
                break;
            case IRCCommand::PART:
                break;
            case IRCCommand::QUIT:
                break;
            case IRCCommand::PING:
                break;
            case IRCCommand::KICK:
                break;
            case IRCCommand::INVITE:
                break;
            case IRCCommand::MODE:
                break;
            case IRCCommand::USERHOST:
                break;
            case IRCCommand::UNKNOW:
                break;
        }
    }
}

static void isValidNick(IRCMessage &msg) {
    if (msg.params.size() < 1) {
        msg.succes = false;
        msg.err.set_value(IRCCode::NEEDMOREPARAMS);
        return;
    }

    if (msg.params[0].length() > getDefaultValue(Defaults::NICKLEN) ||
        msg.params[0].empty()) {
        msg.succes = false;
        msg.err.set_value(IRCCode::ERRONUENICK);
        return;
    }

    const std::string firstAllowed =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[\\]^_`{|}";
    if (firstAllowed.find(msg.params[0][0]) == std::string::npos) {
        msg.succes = false;
        msg.err.set_value(IRCCode::ERRONUENICK);
        msg.errMsg = msg.params[0];
    }

    const std::string allowedChars = firstAllowed + "0123456789-";
    for (size_t i = 1; i < msg.params[0].length(); ++i) {
        if (allowedChars.find(msg.params[0][i]) == std::string::npos) {
            msg.succes = false;
            msg.err.set_value(IRCCode::ERRONUENICK);
            msg.errMsg = msg.params[0];
        }
    }
}

static void isValidUsername(IRCMessage &msg) {
    if (msg.params.size() < 1) {
        msg.succes = false;
        msg.err.set_value(IRCCode::NEEDMOREPARAMS);
        return;
    }

    if (msg.params[0].length() > getDefaultValue(Defaults::USERLEN) ||
        msg.params[0].empty()) {
        msg.succes = false;
        msg.err.set_value(IRCCode::INVALIDUSERNAME);
        return;
    }

    const std::string allowedChars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    for (char c : msg.params[0]) {
        if (allowedChars.find(c) == std::string::npos) {
            msg.succes = false;
            msg.err.set_value(IRCCode::INVALIDUSERNAME);
            msg.errMsg = msg.params[0];
        }
    }
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
