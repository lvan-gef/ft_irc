/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:47:51 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/03/07 22:47:51 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cerrno>
#include <climits>
#include <cstdlib>

#include "../include/utils.hpp"

std::uint16_t toUint16(const std::string &str) {
    char *endptr = nullptr;
    unsigned long int value = strtoul(str.c_str(), &endptr, BASE);

    if (endptr == str || *endptr != '\0') {
        errno = EINVAL;
        return 0;
    } else {
        if (value > UINT16_MAX) {
            errno = ERANGE;
            return 0;
        }
    }

    return static_cast<uint16_t>(value);
}

std::size_t toSizeT(const std::string &str) {
    char *endptr = nullptr;
    unsigned long int value = strtoul(str.c_str(), &endptr, BASE);

    if (endptr == str || *endptr != '\0') {
        errno = EINVAL;
        return 0;
    }

    return static_cast<size_t>(value);
}
