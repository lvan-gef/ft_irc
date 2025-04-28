#ifndef CHATBOT_HPP_
#define CHATBOT_HPP_

#include <cstring>
#include <string>

#include "./Server.hpp"

std::string handleBot(const std::vector<std::string> &params,
                      const std::shared_ptr<Client> &client, Server *server);
bool isBot(const std::string &nickname);

#endif
