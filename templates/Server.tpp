/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.tpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:48:34 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/07 22:48:34 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_TPP
#define SERVER_TPP

#include <iostream>
#include <sstream>
#include <string>

#include <sys/socket.h>

template <typename... Args>
void sendMessage(int fd, const std::string &serverName, const Args &...args) noexcept {
    std::ostringstream oss;

    oss << ":" << serverName << " ";
    (void)std::initializer_list<int>{(oss << args, 0)...};
    oss << "\r\n";

    std::string msg = oss.str();
    std::cout << "Send: " << msg << '\n';
    send(fd, msg.c_str(), msg.length(), 0);
}

#endif // SERVER_TPP
