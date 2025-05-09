#include <cstdint>

#include "../include/Enums.hpp"

bool operator>(const std::uint16_t lhs, Defaults rhs) {
    return lhs > static_cast<std::uint16_t>(rhs);
}

bool operator<(const std::uint16_t lhs, Defaults rhs) {
    return lhs < static_cast<std::uint16_t>(rhs);
}

std::uint16_t getDefaultValue(Defaults rhs) {
    return static_cast<std::uint16_t>(rhs);
}
