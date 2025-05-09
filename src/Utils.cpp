#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../include/Utils.hpp"

std::uint16_t toUint16(const std::string &str) {
    try {
        std::size_t pos = 0;
        const unsigned long value = std::stoul(str, &pos, BASE);

        if (pos != str.size()) {
            errno = EINVAL;
            return 0;
        }

        if (value > UINT16_MAX) {
            errno = ERANGE;
            return 0;
        }

        return static_cast<std::uint16_t>(value);
    } catch (const std::invalid_argument &) {
        errno = EINVAL;
        return 0;
    } catch (const std::out_of_range &) {
        errno = ERANGE;
        return 0;
    }
}

std::size_t toSizeT(const std::string &str) {
    try {
        std::size_t pos = 0;
        const unsigned long value = std::stoul(str, &pos, BASE);

        if (pos != str.size()) {
            errno = EINVAL;
            return 0;
        }

        return static_cast<std::size_t>(value);
    } catch (const std::invalid_argument &) {
        errno = EINVAL;
        return 0;
    } catch (const std::out_of_range &) {
        errno = ERANGE;
        return 0;
    }
}

std::vector<std::string> split(const std::string &str,
                               const std::string &delim) {
    std::vector<std::string> lines;
    std::size_t pos = 0;
    std::size_t prev = 0;

    while ((pos = str.find(delim, prev)) != std::string::npos) {
        try {
            lines.push_back(str.substr(prev, pos - prev));
        } catch (const std::out_of_range &e) {
            std::cerr << "Failed to split: " << e.what() << '\n';
            return std::vector<std::string>{};
        }
        prev = pos + delim.length();
    }

    try {
        lines.push_back(str.substr(prev));
    } catch (const std::out_of_range &e) {
        std::cerr << "Failed to split: " << e.what() << '\n';
        return std::vector<std::string>{};
    }

    return lines;
}
