#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../include/Chatbot.hpp"
#include "../include/Enums.hpp"
#include "../include/Token.hpp"
#include "../include/Utils.hpp"

namespace {
IRCCommand getCommand(const std::string &command) {
    static const std::unordered_map<std::string, IRCCommand> commandMap = {
        {"CAP", IRCCommand::CAP},         {"NICK", IRCCommand::NICK},
        {"USER", IRCCommand::USER},       {"PASS", IRCCommand::PASS},
        {"PRIVMSG", IRCCommand::PRIVMSG}, {"JOIN", IRCCommand::JOIN},
        {"TOPIC", IRCCommand::TOPIC},     {"PART", IRCCommand::PART},
        {"QUIT", IRCCommand::QUIT},       {"PING", IRCCommand::PING},
        {"KICK", IRCCommand::KICK},       {"INVITE", IRCCommand::INVITE},
        {"MODE", IRCCommand::MODE},       {"USERHOST", IRCCommand::USERHOST},
        {"UNKNOW", IRCCommand::UNKNOW},   {"WHOIS", IRCCommand::WHOIS}};

    auto it = commandMap.find(command);
    if (it == commandMap.end()) {
        return IRCCommand::UNKNOW;
    }

    return it->second;
}

void isValidNick(IRCMessage &msg) {
    if (msg.params.size() < 1) {
        msg.err.set_value(IRCCode::NEEDMOREPARAMS);
        msg.errMsg = msg.command;
        msg.succes = false;
        return;
    }

    if (msg.params.size() > 1) {
        msg.err.set_value(IRCCode::ERRONUENICK);
        msg.errMsg = msg.command;
        msg.succes = false;
        return;
    }

    if (msg.params[0].length() > getDefaultValue(Defaults::NICKLEN) ||
        msg.params[0].empty()) {
        msg.err.set_value(IRCCode::ERRONUENICK);
        msg.errMsg = msg.command;
        msg.succes = false;
        return;
    }

    if (isBot(msg.params[0])) {
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

void isValidUsername(IRCMessage &msg) {
    if (msg.params.size() < 4) {
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

void isValidJoin(IRCMessage &msg) {
    if (msg.params.size() < 1) {
        msg.err.set_value(IRCCode::NEEDMOREPARAMS);
        msg.errMsg = msg.command;
        msg.succes = false;
        return;
    }

    std::vector<std::string> channels = split(msg.params[0], ",");
    msg.params.clear();

    for (const std::string &channel : channels) {
        std::cout << "'" << channel << "'" << '\n';
        if (channel[0] != '#') {
            msg.err.set_value(IRCCode::NOSUCHCHANNEL);
            msg.succes = false;
            return;
        }

        msg.params.emplace_back(channel);
    }

}

void isValidTopic(IRCMessage &msg) {
    if (msg.params.size() > 1 && msg.params[0].length() > 0) {
        if (std::any_of(msg.params[0].begin(), msg.params[0].end(),
                        [](char c) { return c < 32 || c == 127; })) {
            msg.err.set_value(IRCCode::TOPIC);
            msg.succes = false;
            msg.errMsg = "Invalid topic";
        }
    }

    if (msg.params.size() > getDefaultValue(Defaults::TOPICLEN)) {
        msg.err.set_value(IRCCode::TOPIC);
        msg.succes = false;
        msg.errMsg = "Topic to long";
    }
}

void isValidMode(IRCMessage &msg) {
    std::cout << "MODE PARAM SIZE: " << msg.params.size() << '\n';
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

    if (msg.params[1][0] == '-' && msg.params[1][1] == 'l') {
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

void validateMessage(std::vector<IRCMessage> &tokens) {
    for (auto &&token : tokens) {
        switch (token.type) {
            case IRCCommand::NICK:
                isValidNick(token);
                break;
            case IRCCommand::USER:
                isValidUsername(token);
                break;
            case IRCCommand::INVITE:
            case IRCCommand::KICK:
            case IRCCommand::PRIVMSG:
                if (token.params.size() < 2) {
                    token.err.set_value(IRCCode::NEEDMOREPARAMS);
                    token.errMsg = token.command;
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
            case IRCCommand::MODE:
                isValidMode(token);
                break;
            case IRCCommand::UNKNOW:
                token.err.set_value(IRCCode::UNKNOWNCOMMAND);
                token.errMsg = token.command;
                token.succes = false;
                break;
            case IRCCommand::CAP:
            case IRCCommand::WHOIS:
                break;
        }
    }
}
} // namespace

std::vector<IRCMessage> parseIRCMessage(const std::string &msg) {
    std::vector<IRCMessage> tokens;
    std::string word;
    std::vector<std::string> lines = split(msg, "\r\n");

    for (const std::string &line : lines) {
        IRCMessage parsed = {};
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
                    try {
                        parsed.params.emplace_back(word.substr(1) + rest);
                    } catch (const std::out_of_range &e) {
                        std::cerr << "Parser failed: " << e.what();
                        return std::vector<IRCMessage>{};
                    }
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
