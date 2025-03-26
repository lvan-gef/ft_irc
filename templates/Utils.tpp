/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.tpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:48:34 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/17 21:24:30 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_TPP
#define UTILS_TPP

#include <cstring>
#include <sstream>
#include <string>

#include <sys/socket.h>

template <typename... Args>
std::string formatMessage(const Args &...args) noexcept {
    std::ostringstream oss;

    (void)std::initializer_list<int>{(oss << args, 0)...};
    oss << "\r\n";

    return oss.str();
}

#endif // UTILS_TPP
