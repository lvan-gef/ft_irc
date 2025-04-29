#ifndef CHATBOT_HPP
#define CHATBOT_HPP

#include <cstring>
#include <string>

#include "./Server.hpp"
#include "./Utils.hpp"

std::string handleBot(const std::vector<std::string> &params,
                      const std::shared_ptr<Client> &client, Server *server);
bool   isBot(std::string nickname);
void botResponseNl(const std::shared_ptr<Client> &client,
    const std::string &response);

#endif // !CHATBOT_HPP
