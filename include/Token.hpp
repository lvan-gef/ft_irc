#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <sys/types.h>
#include <vector>

#include "./Enums.hpp"
#include "./Optional.hpp"

struct IRCMessage {
    std::string prefix;
    std::string command;
    std::vector<std::string> params;
    bool succes;
    Optional<IRCCode> err;
    std::string errMsg;
    IRCCommand type;
};

std::vector<IRCMessage> parseIRCMessage(const std::string &msg);
std::vector<std::string> split(const std::string &str,
                               const std::string &delim);

#endif // !TOKEN_HPP
