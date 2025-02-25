#include <iostream>
#include <sstream>
#include <unordered_map>

#include "../include/Token.hpp"

IRCMessage parseIRCMessage(const std::string &msg) {
    IRCMessage parsed;
    std::istringstream stream(msg);
    std::string word;

    if (msg[0] == ':') {
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

    return parsed;
}

IRCCommand getCommand(const std::string &command) {
    static const std::unordered_map<std::string, IRCCommand> commandMap = {
        {"NICK", IRCCommand::NICK},
        {"USER", IRCCommand::USER},
        {"PASS", IRCCommand::PASS},
        {"PING", IRCCommand::PING},
    };

    auto it = commandMap.find(command);
    if (it == commandMap.end()) {
        return IRCCommand::UNKNOW;
    }

    return it->second;
}

void IRCMessage::print() {
    std::cout << "prefix: '" << prefix << "'" << '\n';
    std::cout << "command: '" << command << "'" << '\n';
    std::cout << "param: ";
    for (const std::string &param : params) {
        std::cout << param << ", ";
    }
    std::cout << '\n';
}
