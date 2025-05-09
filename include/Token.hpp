#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <vector>

#include "./Enums.hpp"
#include "./Optional.hpp"

struct IRCMessage {
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    std::vector<std::string> keys;
    std::string reason;
    bool succes;
    Optional<IRCCode> err;
    std::string errMsg;
    IRCCommand type;

    void debug() const noexcept;
};

std::vector<IRCMessage> parseIRCMessage(const std::string &msg);

#endif // !TOKEN_HPP
