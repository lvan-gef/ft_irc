#ifndef SERVER_TPP
#define SERVER_TPP

#include <sstream>

template <typename... Args>
void Server::_sendMessage(int fd, const Args &...args) noexcept {
    std::ostringstream oss;

    oss << ":" << _serverName << " ";
    (void)std::initializer_list<int>{(oss << args, 0)...};
    oss << "\r\n";

    std::string msg = oss.str();
    send(fd, msg.c_str(), msg.length(), 0);
}

#endif // SERVER_TPP
