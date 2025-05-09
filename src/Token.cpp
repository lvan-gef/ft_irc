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

    const auto it = commandMap.find(command);
    if (it == commandMap.end()) {
        return IRCCommand::UNKNOW;
    }

    return it->second;
}

void isValidNick(IRCMessage &token) {
    if (token.params.size() < 1) {
        token.err.set_value(IRCCode::NEEDMOREPARAMS);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    if (token.params.size() > 1) {
        token.err.set_value(IRCCode::ERRONUENICK);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    if (token.params[0].length() > getDefaultValue(Defaults::NICKLEN) ||
        token.params[0].empty()) {
        token.err.set_value(IRCCode::ERRONUENICK);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    if (isBot(token.params[0])) {
        token.succes = false;
        token.err.set_value(IRCCode::ERRONUENICK);
        return;
    }

    const std::string firstAllowed =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[\\]^_`{|}";
    if (firstAllowed.find(token.params[0][0]) == std::string::npos) {
        token.succes = false;
        token.err.set_value(IRCCode::ERRONUENICK);
        token.errMsg = token.params[0];
    }

    const std::string allowedChars = firstAllowed + "0123456789-";
    for (size_t i = 1; i < token.params[0].length(); ++i) {
        if (allowedChars.find(token.params[0][i]) == std::string::npos) {
            token.succes = false;
            token.err.set_value(IRCCode::ERRONUENICK);
            token.errMsg = token.params[0];
        }
    }
}

void isValidUsername(IRCMessage &token) {
    if (token.params.size() < 4) {
        token.err.set_value(IRCCode::NEEDMOREPARAMS);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    if (token.params[0].length() > getDefaultValue(Defaults::USERLEN) ||
        token.params[0].empty()) {
        token.err.set_value(IRCCode::INVALIDUSERNAME);
        token.succes = false;
        return;
    }

    const std::string allowedChars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    for (const char c : token.params[0]) {
        if (allowedChars.find(c) == std::string::npos) {
            token.succes = false;
            token.err.set_value(IRCCode::INVALIDUSERNAME);
            token.errMsg = token.params[0];
        }
    }
}

void isValidTopic(IRCMessage &token) {
    if (token.params.size() > 1 && token.params[0].length() > 0) {
        if (std::any_of(token.params[0].begin(), token.params[0].end(),
                        [](const char c) { return c < 32 || c == 127; })) {
            token.err.set_value(IRCCode::TOPIC);
            token.succes = false;
            token.errMsg = "Invalid topic";
        }
    }

    if (token.params.size() > getDefaultValue(Defaults::TOPICLEN)) {
        token.err.set_value(IRCCode::TOPIC);
        token.succes = false;
        token.errMsg = "Topic to long";
    }
}

void isValidMode(IRCMessage &token) {
    if (token.params.size() == 1) {
        return;
    }

    if (token.params.size() < 2) {
        token.err.set_value(IRCCode::NEEDMOREPARAMS);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    if (token.params[1].length() < 2) {
        token.err.set_value(IRCCode::UNKNOWNMODEFLAG);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    const std::string allowedModes = "itkol";
    const std::string firstAllowed = "+-";
    if (firstAllowed.find(token.params[1][0]) == std::string::npos ||
        allowedModes.find(token.params[1][1]) == std::string::npos) {
        token.err.set_value(IRCCode::UNKNOWMODE);
        token.errMsg = "MODE " + token.params[1];
        token.succes = false;
        return;
    }

    if (token.params[1][0] == '-' && token.params[1][1] == 'l') {
        return;
    }

    if (token.params[1][1] == 'l' || token.params[1][1] == 'k' ||
        token.params[1][1] == 'o') {
        if (token.params.size() < 3) {
            token.err.set_value(IRCCode::NEEDMOREPARAMS);
            token.errMsg = "MODE " + std::string(1, token.params[1][1]);
            token.succes = false;
            return;
        }
    }
}

void isValidJoin(IRCMessage &token) {
    if (token.params.size() < 1) {
        token.err.set_value(IRCCode::NEEDMOREPARAMS);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    if (token.params.size() > 1) {
        const std::vector<std::string> keys = split(token.params[1], ",");
        for (const std::string &key : keys) {
            token.keys.emplace_back(key);
        }
    }

    const std::vector<std::string> channels = split(token.params[0], ",");
    token.params.clear();

    for (const std::string &channel : channels) {
        if (channel.size() < 2 || channel[0] != '#' ||
            channel.size() >
                static_cast<std::size_t>(Defaults::MAXCHANNELLEN)) {
            token.err.set_value(IRCCode::NOSUCHCHANNEL);
            token.errMsg = token.command + " " + channel;
            token.succes = false;
            return;
        }

        token.params.emplace_back(channel);
    }
}

void isValidPart(IRCMessage &token) {
    if (token.params.size() < 1) {
        token.err.set_value(IRCCode::NEEDMOREPARAMS);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    token.reason = "Bye";
    const std::vector<std::string> parts = split(token.params[0], ",");
    token.params.clear();

    for (const std::string &part : parts) {
        if (part.size() < 2 || part[0] != '#' ||
            part.size() > static_cast<std::size_t>(Defaults::MAXCHANNELLEN)) {
            token.err.set_value(IRCCode::NOSUCHCHANNEL);
            token.errMsg = token.command + " " + part;
            token.succes = false;
            return;
        }

        token.params.emplace_back(part);
    }
}

void isValidKick(IRCMessage &token) {
    token.debug();
    if (token.params.size() < 2) {
        token.err.set_value(IRCCode::NEEDMOREPARAMS);
        token.errMsg = token.command;
        token.succes = false;
        return;
    }

    const std::vector<std::string> keys = split(token.params[1], ",");
    for (const std::string &key : keys) {
        token.keys.emplace_back(key);
    }

    token.reason = "Kicked";
    if (token.params.size() > 2) {
        token.reason = token.params[2];
    }

    const std::vector<std::string> channels = split(token.params[0], ",");
    token.params.clear();

    for (const std::string &channel : channels) {
        token.params.emplace_back(channel);
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
            case IRCCommand::KICK:
                isValidKick(token);
                break;
            case IRCCommand::INVITE:
            case IRCCommand::PRIVMSG:
                if (token.params.size() < 2) {
                    token.err.set_value(IRCCode::NEEDMOREPARAMS);
                    token.errMsg = token.command;
                    token.succes = false;
                }
                break;
            case IRCCommand::TOPIC:
                isValidTopic(token);
                break;
            case IRCCommand::JOIN:
                isValidJoin(token);
                break;
            case IRCCommand::PART:
                isValidPart(token);
                break;
            case IRCCommand::PASS:
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

void IRCMessage::debug() const noexcept {
    std::cout << "------------------" << '\n';
    std::cout << "prefix : " << this->prefix << '\n';
    std::cout << "command: " << this->command << '\n';
    std::cout << "params : " << '\n';
    for (const std::string &param : this->params) {
        std::cout << "    " << param << '\n';
    }
    std::cout << "succes : " << this->succes << '\n';
    std::cout << "err msg: " << this->errMsg << '\n';
    // std::cout << "type   : " << this->type << '\n';
    std::cout << "------------------" << '\n';
}
