#include <climits>
#include <cstdlib>
#include <cerrno>

#include "../include/utils.hpp"

int toInt(const char *str) {
    char *endptr = nullptr;
    long int value = strtol(str, &endptr, BASE);

    if (endptr == str || *endptr != '\0') {
        errno = EINVAL;
        return 0;
    } else {
        if (value > INT_MAX || value < 0) {
            errno = ERANGE;
            return 0;
        }
    }

    return static_cast<int>(value);
}
