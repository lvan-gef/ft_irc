#ifndef ENUMS_HPP
#define ENUMS_HPP

#include <cstdint>
#include <limits>

#include <sys/socket.h>

enum class Defaults : std::uint16_t {
    LOWEST_PORT = 1024,
    HIGHEST_PORT = 65535,
    EVENT_SIZE = 2048,
    MAX_CONNECTIONS = SOMAXCONN,
    READ_SIZE = 512,
    INTERVAL = 1000,
    TIMEOUT = 360,
    USERLIMIT = std::numeric_limits<uint16_t>::max(),
    NICKLEN = 9,
    USERLEN = 10,
    TOPICLEN = 512,
    MAXMSGLEN = 512,
};
bool operator>(std::uint16_t lhs, Defaults rhs);
bool operator<(std::uint16_t lhs, Defaults rhs);
std::uint16_t getDefaultValue(Defaults rhs);

enum class ChatBot : std::int8_t {
    CHANNELS,
    HELLO,
    JOKE,
    HELP,
    PING,
    QUOTE,
    WEATHER,
    WEATHER_TOO_FEW,
    WEATHER_TOO_MANY,
    UNKNOWN
};

enum class ChannelMode : std::uint8_t {
    INVITE_ONLY,
    TOPIC_PROTECTED,
    PASSWORD_PROTECTED,
    OPERATOR,
    USER_LIMIT,
};

enum class ChannelCommand : std::uint8_t {
    MODE_I = 'i',
    MODE_T = 't',
    MODE_K = 'k',
    MODE_O = 'o',
    MODE_L = 'l'
};

enum class IRCCommand : std::uint8_t {
    CAP,
    NICK,
    USER,
    PASS,
    PRIVMSG,
    JOIN,
    TOPIC,
    PART,
    QUIT,
    PING,
    KICK,
    INVITE,
    MODE,
    USERHOST,
    WHOIS,
    UNKNOW
};

enum class IRCCode : std::int16_t {
    WELCOME = 1,
    YOURHOST = 2,
    CREATED = 3,
    MYINFO = 4,
    ISUPPORT = 5,
    USERHOST = 302,
    RPL_WHOISUSER = 311,
    RPL_WHOISSERVER = 312,
    RPL_ENDOFWHOIS = 318,
    CHANNELMODEIS = 324,
    TOPIC = 332,
    INVITING = 341,
    NAMREPLY = 353,
    ENDOFNAMES = 366,
    MOTD = 372,
    MOTDSTART = 375,
    ENDOFMOTD = 376,
    NOSUCHNICK = 401,
    NOSUCHCHANNEL = 403,
    CANNOTSENDTOCHAN = 404,
    TOMANYCHANNELS = 405,
    NORECIPIENT = 411,
    NOTEXTTOSEND = 412,
    INPUTTOOLONG = 417,
    UNKNOWNCOMMAND = 421,
    NONICK = 431,
    ERRONUENICK = 432,
    NICKINUSE = 433,
    USERNOTINCHANNEL = 441,
    NOTOCHANNEL = 442,
    USERONCHANNEL = 443,
    NOTREGISTERED = 451,
    NEEDMOREPARAMS = 461,
    ALREADYREGISTERED = 462,
    PASSWDMISMATCH = 464,
    KEYSET = 467,
    INVALIDUSERNAME = 468,
    CHANNELISFULL = 471,
    UNKNOWMODE = 472,
    INVITEONLYCHAN = 473,
    BADCHANNELKEY = 475,
    NOPRIVILEGES = 481,
    CHANOPRIVSNEEDED = 482,
    UNKNOWNMODEFLAG = 501,
    USERSDONTMATCH = 502,
    INVALIDMODEPARAM = 696,
    INVITENOTICE = 992, // for my own use
    MODE = 993,         // for my own use
    TOPICNOTICE = 994,  // for my own use
    KICK = 995,         // for my own use
    PART = 996,         // for my own use
    JOIN = 997,         // for my own use
    NICKCHANGED = 998,  // for my own use
    PRIVMSG = 999,      // for my own use
};

#endif // ENUMS_HPP
