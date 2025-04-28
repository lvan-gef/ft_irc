#ifndef CHATBOT_HPP_
#define CHATBOT_HPP_

#include "Server.hpp"
#include <cstring>
#include <string>

std::string handleBot(const std::vector<std::string> &params,
                      const std::shared_ptr<Client> &client, Server *server);
bool isBot(std::string nickname);

#endif
