/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.hpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:48:14 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/27 21:43:36 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <memory>
#include <string>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"

#define BASE 10
#define NAME "codamirc"
#define serverName "codamirc.local"
#define serverVersion "0.4.0"
#define serverCreated "Mon Feb 19 2025 at 10:00:00 UTC"

std::uint16_t toUint16(const std::string &str);
std::size_t toSizeT(const std::string &str);

template <typename... Args>
std::string formatMessage(const Args &...args) noexcept;

void handleMsg(IRCCode code, const std::shared_ptr<Client> &client,
               const std::string &value, const std::string &msg);

#include "../templates/Utils.tpp"

#endif // UTILS_HPP
