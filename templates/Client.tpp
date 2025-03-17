/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Client.tpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:48:34 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/13 17:41:51 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_TPP
#define CLIENT_TPP

#include <sstream>
#include <string>
#include <cstring>

#include <sys/socket.h>

template <typename... Args>
void Client::appendMessageToQue(int epollFD, const std::string &serverName,
                                const Args &...args) noexcept {
    std::ostringstream oss;

    oss << ":" << serverName << " ";
    (void)std::initializer_list<int>{(oss << args, 0)...};
    oss << "\r\n";

    std::string msg = oss.str();
    _messages.emplace(msg);
    std::cout << "Message queued: " << msg << std::endl;

    epoll_event ev = getEvent();
    ev.events = EPOLLIN | EPOLLOUT;
    std::cout << epollFD << " " << getFD() << '\n';
    if (epoll_ctl(epollFD, EPOLL_CTL_MOD, getFD(), &ev) == -1) {
        std::cerr << "epoll_ctl failed: " << strerror(errno) << '\n';
    }
}

#endif // CLIENT_TPP
