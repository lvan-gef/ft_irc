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

#include <sstream>

template <typename... Args>
void Server::_sendMessage(int fd, const Args &...args) noexcept {
    std::ostringstream oss;

    oss << ":" << _serverName << " ";
    (void)std::initializer_list<int>{(oss << args, 0)...};
    oss << "\r\n";

    std::string msg = oss.str();
    std::cerr << "Send: " << msg << '\n';
    send(fd, msg.c_str(), msg.length(), 0);
}

#endif // SERVER_TPP
