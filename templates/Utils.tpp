#ifndef UTILS_TPP
#define UTILS_TPP

#include <sstream>
#include <string>

template <typename... Args>
std::string formatMessage(const Args &...args) noexcept {
    std::ostringstream oss;

    (void)std::initializer_list<int>{(oss << args, 0)...};
    oss << "\r\n";

    return oss.str();
}

#endif // UTILS_TPP
