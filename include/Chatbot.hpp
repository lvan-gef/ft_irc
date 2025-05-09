#ifndef CHATBOT_HPP
#define CHATBOT_HPP

#include <string>
#include <vector>

#include <sys/epoll.h>

#include "./Server.hpp"

std::string handleBot(const std::vector<std::string> &params,
                      const std::shared_ptr<Client> &client, Server *server);
bool isBot(const std::string &nickname);
void botResponseNl(const std::shared_ptr<Client> &client,
                   const std::string &response);
bool handleSendApi(ApiRequest &api, epoll_event event, int epoll_fd);
void handleRecvApi(ApiRequest &api);

#endif // !CHATBOT_HPP
