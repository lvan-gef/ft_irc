/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Enum.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/26 01:01:19 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/26 01:01:19 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cstdint>

#include "../include/Enums.hpp"

bool operator>(std::uint16_t lhs, Defaults rhs) {
    return lhs > static_cast<std::uint16_t>(rhs);
}

bool operator<(std::uint16_t lhs, Defaults rhs) {
    return lhs < static_cast<std::uint16_t>(rhs);
}

constexpr std::uint16_t getDefaultValue(Defaults rhs) {
    return static_cast<std::uint16_t>(rhs);
}
