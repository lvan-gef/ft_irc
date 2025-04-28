/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/03/07 22:47:51 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/04/02 16:52:22 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../include/Utils.hpp"

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

std::vector<std::string> split(const std::string &s,
                               const std::string &delimiter) {
    std::vector<std::string> lines;
    size_t pos = 0;
    size_t prev = 0;

    while ((pos = s.find(delimiter, prev)) != std::string::npos) {
        try {
            lines.push_back(s.substr(prev, pos - prev));
        } catch (const std::out_of_range &e) {
            std::cerr << "Failed to split: " << e.what() << '\n';
            return std::vector<std::string>{};
        }
        prev = pos + delimiter.length();
    }

    lines.push_back(s.substr(prev));

    return lines;
}
