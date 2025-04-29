#include <cstring>
#include <memory>

#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Token.hpp"
#include "../include/Utils.hpp"

void Server::_handleCommand(const IRCMessage &token,
                            const std::shared_ptr<Client> &client) noexcept {

    switch (token.type) {
        case IRCCommand::CAP:
            break;
        case IRCCommand::NICK:
            if (!client->getPasswordBit()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleNickname(token, client);
        case IRCCommand::USER:
            if (!client->getPasswordBit()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleUsername(token, client);
        case IRCCommand::PASS:
            return _handlePassword(token, client);
        case IRCCommand::PRIVMSG:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handlePriv(token, client);
        case IRCCommand::JOIN:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleJoin(token, client);
        case IRCCommand::TOPIC:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleTopic(token, client);
        case IRCCommand::PART:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handlePart(token, client);
        case IRCCommand::QUIT:
            _removeClient(client);
            break;
        case IRCCommand::PING:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handlePing(token, client);
        case IRCCommand::KICK:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleKick(token, client);
        case IRCCommand::INVITE:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleInvite(token, client);
        case IRCCommand::MODE:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleMode(token, client);
        case IRCCommand::USERHOST:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleUserhost(token, client);
        case IRCCommand::WHOIS:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleWhois(token, client);
        case IRCCommand::UNKNOW:
            if (!client->isRegistered()) {
                client->setDisconnect();
                return handleMsg(IRCCode::NOTREGISTERED, client, "", "");
            }
            return _handleUnkown(token, client);
    }
}
