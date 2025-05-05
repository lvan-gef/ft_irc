#include <cerrno>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "../include/Utils.hpp"

std::uint16_t toUint16(const std::string &str) {
    char *endptr = nullptr;
    const char *c_str = str.c_str();
    unsigned long int value = strtoul(str.c_str(), &endptr, BASE);

    if (endptr == c_str || *endptr != '\0') {
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

    try {
        lines.push_back(s.substr(prev));
    } catch (const std::out_of_range &e) {
        std::cerr << "Failed to split: " << e.what() << '\n';
        return std::vector<std::string>{};
    }

    return lines;
}
