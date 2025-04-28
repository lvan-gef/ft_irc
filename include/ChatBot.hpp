#ifndef CHATBOT_HPP_
#define CHATBOT_HPP_

#include "Enums.hpp"
#include "Utils.hpp"
#include "Token.hpp"
#include <string>
#include <algorithm>
#include <cstring>
#include "Server.hpp"

//std::string getWeatherDirectly(const std::string& location);
std::string handleBot(std::vector<std::string> params,const std::shared_ptr<Client> &client, Server *server);
bool isBot(std::string nickname);

#endif