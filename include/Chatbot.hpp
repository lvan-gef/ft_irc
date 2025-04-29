#ifndef CHATBOT_HPP
#define CHATBOT_HPP

#include <cstring>
#include <string>

#include "./Server.hpp"

std::string handleBot(const std::vector<std::string> &params,
                      const std::shared_ptr<Client> &client, Server *server);

#endif // !CHATBOT_HPP
