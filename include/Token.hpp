#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <vector>

struct IRCMessage {
    std::string prefix;
    std::string command;
    std::vector<std::string> params;

    void print();
};

enum class IRCCommand {
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

IRCMessage parseIRCMessage(const std::string &msg);
IRCCommand getCommand(const std::string &command);

#endif // !TOKEN_HPP

