#include <cstdint>
#include <memory>
#include <string>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"
#include "../include/Utils.hpp"

void handleMsg(IRCCode code, const std::shared_ptr<Client> &client,
               const std::string &value, const std::string &msg) noexcept {
    std::string ircCode = std::to_string(static_cast<std::uint16_t>(code));
    if (ircCode.length() < 3) {
        ircCode.insert(0, 3 - ircCode.length(), '0');
    }

    switch (code) {
        case IRCCode::WELCOME:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(),
                " :Welcome to the IRCCodam Network ", client->getFullID()));
            break;
        case IRCCode::YOURHOST:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), " :Your host is ",
                              serverName, ", running version ", serverVersion));
            break;
        case IRCCode::CREATED:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(),
                " :This server was created ", msg));
            break;
        case IRCCode::MYINFO:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(), " ",
                serverName, " ", serverVersion, " ", msg));
            break;
        case IRCCode::ISUPPORT:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(),
                " CHANMODES=i,t,k,o,l ", "CHANTYPES=# ", "PREFIX=(o)@ ",
                "STATUSMSG=@ ", "NICKLEN=", getDefaultValue(Defaults::NICKLEN),
                " NETWORK=", NAME, " :are supported by this server"));
            break;
        case IRCCode::USERHOST:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), " :", msg));
            break;
        case IRCCode::CHANNELMODEIS:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), " ", value, " ", msg));
            break;
        case IRCCode::TOPIC:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), " ", value, " ", msg));
            break;
        case IRCCode::INVITING:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname() + " " + value, " " + msg));
            break;
        case IRCCode::NAMREPLY:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), " = ", value, " :", msg));
            break;
        case IRCCode::ENDOFNAMES:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(), "  ",
                value, " :End of /NAMES list"));
            break;
        case IRCCode::MOTD:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " :", msg));
            break;
        case IRCCode::MOTDSTART:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(),
                " :- Message of the Day -"));
            break;
        case IRCCode::ENDOFMOTD:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), " :End of /MOTD command"));
            break;
        case IRCCode::NOSUCHNICK:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(), " ",
                value, " :No such nick/channel"));
            break;
        case IRCCode::NOSUCHCHANNEL:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(), " ",
                value, " :No such channel"));
            break;
        case IRCCode::CANNOTSENDTOCHAN:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, value,
                              " :Cannot send to channel"));
            break;
        case IRCCode::TOMANYCHANNELS:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, value,
                              " :You have joined too many channels"));
            break;
        case IRCCode::NORECIPIENT:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, value,
                              " :No recipient given ", msg));
            break;
        case IRCCode::NOTEXTTOSEND:
            client->appendMessageToQue(formatMessage(":", serverName, " ",
                                                     ircCode, " ", value,
                                                     " :No text to send"));
            break;
        case IRCCode::INPUTTOOLONG:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), " :Input was too long"));
            break;
        case IRCCode::UNKNOWNCOMMAND:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), " ", value, " :", msg));
            break;
        case IRCCode::NONICK:
            client->appendMessageToQue(formatMessage(":", serverName, " ",
                                                     ircCode, " * ", value,
                                                     " :No nickname given"));
            break;
        case IRCCode::ERRONUENICK:
            client->appendMessageToQue(formatMessage(":", serverName, " ",
                                                     ircCode, " * ", value,
                                                     " :Erronues nickname"));
            break;
        case IRCCode::NICKINUSE: {
            std::string clientNick = " * ";
            if (client->getNickname() != "") {
                clientNick = " " + client->getNickname() + " ";
            }
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, clientNick, value,
                              " :Nickname is already in use"));
            break;
        }
        case IRCCode::USERNOTINCHANNEL:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(), " ",
                value, " ", msg, " :They aren't on that channel"));
            break;
        case IRCCode::NOTOCHANNEL:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, value,
                              " :You're not on that channel"));
            break;
        case IRCCode::USERONCHANNEL:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ", value, " ",
                              msg, " :is already in channel"));
            break;
        case IRCCode::NOTREGISTERED:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " :You have not registered"));
            break;
        case IRCCode::NEEDMOREPARAMS:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(), " ",
                value, " :Not enough parameters"));
            break;
        case IRCCode::ALREADYREGISTERED:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " :You may not reregister"));
            break;
        case IRCCode::PASSWDMISMATCH:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " *", " :Password incorrect"));
            break;
        case IRCCode::KEYSET:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, value,
                              " :Channel key already set"));
            break;
        case IRCCode::INVALIDUSERNAME:
            client->appendMessageToQue(formatMessage(":", serverName, " ",
                                                     ircCode, " * ", value,
                                                     " :Erronues username"));
            if (client->isRegistered() != true) {
                client->setDisconnect();
            }
            break;
        case IRCCode::CHANNELISFULL:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ", value,
                              " :Cannot join channel (+l)"));
            break;
        case IRCCode::UNKNOWMODE:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ", value,
                              " :is unknown mode char to me"));
            break;
        case IRCCode::INVITEONLYCHAN:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ", value,
                              " :Cannot join channel (+i)"));
            break;
        case IRCCode::BADCHANNELKEY:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(),
                +" " + value, " :Cannot join channel (+k) - bad key"));
            break;
        case IRCCode::NOPRIVILEGES:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode,
                " :Permission Denied- You're not an IRC operator"));
            break;
        case IRCCode::CHANOPRIVSNEEDED:
            client->appendMessageToQue(formatMessage(
                ":", serverName, " ", ircCode, " ", client->getNickname(), " ",
                value, " :You're not channel operator"));
            break;
        case IRCCode::UNKNOWNMODEFLAG:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getFullID(), " :Unknown MODE flag"));
            break;
        case IRCCode::USERSDONTMATCH:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode,
                              " :Cant change mode for other users"));
            break;
        case IRCCode::INVALIDMODEPARAM:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " ",
                              client->getNickname(), value, " : ", msg));
            break;
        case IRCCode::MODE:
            client->appendMessageToQue(
                formatMessage(":", value, " MODE ", msg));
            break;
        case IRCCode::TOPICNOTICE:
            client->appendMessageToQue(formatMessage(":", value, msg));
            break;
        case IRCCode::KICK:
            client->appendMessageToQue(
                formatMessage(":", value, " KICK ", msg));
            break;
        case IRCCode::PART:
            client->appendMessageToQue(
                formatMessage(":", value, " PART ", msg));
            break;
        case IRCCode::JOIN:
            client->appendMessageToQue(
                formatMessage(":", value, " JOIN ", msg));
            break;
        case IRCCode::NICKCHANGED:
            client->appendMessageToQue(
                formatMessage(":", value, " NICK ", msg));
            break;
        case IRCCode::PRIVMSG:
            client->appendMessageToQue(
                formatMessage(":", value, " PRIVMSG ", msg));
            break;
        case IRCCode::RPL_WHOISUSER:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, msg));
            break;
        case IRCCode::RPL_WHOISSERVER:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " :", msg));
            break;
        case IRCCode::RPL_ENDOFWHOIS:
            client->appendMessageToQue(formatMessage(":", serverName, " ",
                                                     ircCode, " ", msg,
                                                     " :End of WHOIS list"));
            break;
        case IRCCode::INVITENOTICE:
            client->appendMessageToQue(
                formatMessage(":", serverName, " ", ircCode, " :", msg));

            break;
    }
}
