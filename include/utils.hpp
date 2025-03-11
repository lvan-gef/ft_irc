/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:48:14 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/07 22:48:14 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <string>

#define BASE 10

std::uint16_t toUint16(const std::string &str);

template <typename... Args>
void sendMessage(int fd, const std::string &serverName, const Args &...args) noexcept;

#include "../templates/Server.tpp"

#endif // UTILS_HPP
