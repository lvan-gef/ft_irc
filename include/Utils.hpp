#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstdint>
#include <memory>
#include <string>

#include "../include/Client.hpp"
#include "../include/Enums.hpp"

constexpr int BASE = 10;
constexpr const char *NAME = "codamirc";
constexpr const char *serverName = "codamirc.local";
constexpr const char *serverVersion = "0.5.1";

std::uint16_t toUint16(const std::string &str);
std::size_t toSizeT(const std::string &str);

template <typename... Args>
std::string formatMessage(const Args &...args) noexcept;

void handleMsg(IRCCode code, const std::shared_ptr<Client> &client,
               const std::string &value, const std::string &msg) noexcept;

std::vector<std::string> split(const std::string &s,
                               const std::string &delimiter);

#include "../templates/Utils.tpp"

#endif // UTILS_HPP
