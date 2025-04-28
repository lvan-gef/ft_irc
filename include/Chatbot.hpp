#ifndef CHATBOT_HPP_
#define CHATBOT_HPP_

#include "Enums.hpp"
#include "Server.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cstring>
#include <string>

// std::string getWeatherDirectly(const std::string& location);
std::string handleBot(std::vector<std::string> params,
                      const std::shared_ptr<Client> &client, Server *server);
bool isBot(std::string nickname);

#endif
