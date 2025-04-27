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
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/Enums.hpp"
#include "../include/Token.hpp"

static IRCCommand getCommand(const std::string &command);
static void validateMessage(std::vector<IRCMessage> &tokens);
static void isValidNick(IRCMessage &msg);
static void isValidUsername(IRCMessage &msg);
static void isValidJoin(IRCMessage &msg);
static void isValidTopic(IRCMessage &msg);
static void isValidMode(IRCMessage &msg);

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
            case IRCCommand::NICK:
                isValidNick(token);
                break;
            case IRCCommand::USER:
                isValidUsername(token);
                break;
            case IRCCommand::PRIVMSG:
                if (token.params.size() < 2) {
                    token.err.set_value(IRCCode::NEEDMOREPARAMS);
                    token.succes = false;
                }
                break;
            case IRCCommand::JOIN:
                isValidJoin(token);
                break;
            case IRCCommand::TOPIC:
                isValidTopic(token);
                break;
            case IRCCommand::PASS:
            case IRCCommand::PART:
            case IRCCommand::QUIT:
            case IRCCommand::PING:
            case IRCCommand::USERHOST:
                if (token.params.size() < 1) {
                    token.err.set_value(IRCCode::NEEDMOREPARAMS);
                    token.errMsg = token.command;
                    token.succes = false;
                }
                break;
            case IRCCommand::KICK:
            case IRCCommand::INVITE:
                if (token.params.size() < 3) {
                    token.err.set_value(IRCCode::NEEDMOREPARAMS);
                    token.errMsg = token.command;
                    token.succes = false;
                }
                break;
            case IRCCommand::MODE:
                isValidMode(token);
                break;
            case IRCCommand::CAP:
            case IRCCommand::UNKNOW:
                break;
        }
    }
}

static void isValidNick(IRCMessage &msg) {
    if (msg.params.size() < 1) {
        msg.err.set_value(IRCCode::NEEDMOREPARAMS);
        msg.errMsg = msg.command;
        msg.succes = false;
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
        msg.err.set_value(IRCCode::NEEDMOREPARAMS);
        msg.errMsg = msg.command;
        msg.succes = false;
        return;
    }

    if (msg.params[0].length() > getDefaultValue(Defaults::USERLEN) ||
        msg.params[0].empty()) {
        msg.err.set_value(IRCCode::INVALIDUSERNAME);
        msg.succes = false;
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

static void isValidJoin(IRCMessage &msg) {
    if (msg.params.size() < 1) {
        msg.err.set_value(IRCCode::NEEDMOREPARAMS);
        msg.errMsg = msg.command;
        msg.succes = false;
        return;
    }

    if (msg.params[0][0] != '#') {
        msg.err.set_value(IRCCode::NOSUCHCHANNEL);
        msg.succes = false;
        return;
    }
}

static void isValidTopic(IRCMessage &msg) {
    if (msg.params.size() > 1 && msg.params[0].length() > 0) {
        for (char c : msg.params[0]) {
            if (c < 32 || c == 127) {
                msg.err.set_value(IRCCode::TOPIC);
                msg.succes = false;
                msg.errMsg = "Invalid topic";
                break;
            }
        }
    }

    if (msg.params.size() > getDefaultValue(Defaults::TOPICLEN)) {
        msg.err.set_value(IRCCode::TOPIC);
        msg.succes = false;
        msg.errMsg = "Topic to long";
    }
}

static void isValidMode(IRCMessage &msg) {
    msg.print();

    if (msg.params.size() == 1) {
        return;
    }

    if (msg.params.size() < 2) {
        msg.err.set_value(IRCCode::NEEDMOREPARAMS);
        msg.errMsg = msg.command;
        msg.succes = false;
        return;
    }

    if (msg.params[1].length() < 2) {
        msg.err.set_value(IRCCode::UNKNOWNMODEFLAG);
        msg.errMsg = msg.command;
        msg.succes = false;
        return;
    }

    const std::string allowedModes = "itkol";
    const std::string firstAllowed = "+-";
    if (firstAllowed.find(msg.params[1][0]) == std::string::npos ||
        allowedModes.find(msg.params[1][1]) == std::string::npos) {
        msg.err.set_value(IRCCode::UNKNOWMODE);
        msg.errMsg = "MODE " + msg.params[1];
        msg.succes = false;
        return;
    }

    if (msg.params[1][1] == 'l' || msg.params[1][1] == 'k' ||
        msg.params[1][1] == 'o') {
        if (msg.params.size() < 3) {
            msg.err.set_value(IRCCode::NEEDMOREPARAMS);
            msg.errMsg = "MODE " + std::string(1, msg.params[1][1]);
            msg.succes = false;
            return;
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
