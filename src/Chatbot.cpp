#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unordered_map>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include "../include/Chatbot.hpp"
#include "../include/Enums.hpp"
#include "../include/Server.hpp"
#include "../include/Utils.hpp"

namespace {
std::string findVal(const std::string &json, const std::string &key) {
    std::string key_to_find = "\"" + key + "\":";
    size_t key_pos = json.find(key_to_find);

    if (key_pos == std::string::npos) {
        return "Key not found";
    }

    size_t value_start = key_pos + key_to_find.length();

    while (value_start < json.length() && isspace(json[value_start])) {
        value_start++;
    }

    if (value_start >= json.length()) {
        return "No value found after key";
    }

    if (json[value_start] == '"') {
        size_t value_end = json.find('"', value_start + 1);
        if (value_end != std::string::npos) {
            try {
                return json.substr(value_start + 1,
                                   value_end - value_start - 1);
            } catch (const std::out_of_range &e) {
                std::cerr << "Failed to substr: " << e.what() << '\n';
                return "Internal server error";
            }
        }
    } else if (isdigit(json[value_start]) || json[value_start] == '-') {
        size_t value_end = value_start;
        while (value_end < json.length() &&
               (isdigit(json[value_end]) || json[value_end] == '.' ||
                json[value_end] == '-')) {
            value_end++;
        }
        try {
            return json.substr(value_start, value_end - value_start);
        } catch (const std::out_of_range &e) {
            std::cerr << "Failed to substr: " << e.what() << '\n';
            return "Internal server error";
        }
    }

    return "Couldn't parse";
}

std::string extractWeather(const std::string &json) {
    if (json.empty() || json.rfind("Error:", 0) == 0) {
        return "Json empty or error";
    }

    std::string temp_c = findVal(json, "temp_c");
    std::string description = findVal(json, "text");
    std::string location = findVal(json, "name");
    std::string country = findVal(json, "country");

    if (temp_c.empty() || description.empty() || location.empty()) {
        return "Error: Could not parse weather data from API response.";
    }

    std::string result = location;
    if (!country.empty()) {
        result += ", " + country;
    }
    result += ": " + temp_c + "°C, " + description;

    return result;
}

int getApiInfo(const char *hostname, const char *port) {
    struct addrinfo hints {
    }, *res = nullptr, *p = nullptr;
    int sockfd = -1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int addr = getaddrinfo(hostname, port, &hints, &res);
    if (addr != 0) {
        std::cerr << "Could not resolve API hostname" << '\n';
        return -1;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            continue;
        }

        if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
            close(sockfd);
            continue;
        }

        int status = connect(sockfd, p->ai_addr, p->ai_addrlen);
        if (status == 0 || (status == -1 && errno == EINPROGRESS)) {
            break;
        } else {
            close(sockfd);
            sockfd = -1;
        }
    }

    freeaddrinfo(res);

    if (sockfd == -1) {
        std::cerr << "Failed to create API connection socket" << '\n';
        return -1;
    }

    return sockfd;
}

std::string getJoke() {
    static const std::vector<std::string> jokes = {
        "Why did the scarecrow win an award? Because he was outstanding in his "
        "field!",
        "Algorithm: A word used by programmers when they don't want to explain "
        "how their code works.",
        "Debugging is like being the detective in a crime movie where you're "
        "also the murderer at the same time.",
        "The six stages of debugging:\n1. That can't happen.\n2. That doesn't "
        "happen on my machine.\n3."
        " That shouldn't happen.\n4. Why does that happen?\n5. Oh, I see.\n6. "
        "How did that ever work?",
        "My husband and I were happy for 20 years. And then we met.",
        "What kind of doctor is Dr. Pepper?, \nHe's a fizzician.",
        "Why did the coffee file a police report? \nIt got mugged.",
        "What do you call a fish wearing a bowtie? \nSofishticated.",
        "I told my suitcase we're not going on vacation — now it's dealing "
        "with emotional baggage."};

    int index = std::rand() % static_cast<int>(jokes.size());
    return jokes[static_cast<std::vector<std::string>::size_type>(index)];
}

std::string getQuote() {

    static const std::vector<std::string> quotes = {
        "\"Your friend is your needs answered.\" - Kahlil Gibran",
        "\"How is it possible to find meaning in a finite world, given my "
        "waist and shirt size?\" - Woody Allen",
        "\"A true friend freely, advises justly, assists readily, adventures "
        "boldly, takes all patiently,"
        " defends courageously, and continues a friend unchangeably.\" - "
        "William C. Menninger",
        "\"Imagination is the highest kite one can fly.\" - Lauren Bacall",
        "\"The personal life deeply lived always expands into truths beyond "
        "itself.\" - Anaïs Nin",
        "\"Without leaps of imagination, or dreaming, we lose the excitement "
        "of possibilities. Dreaming, after all, is a form of planning.\" - "
        "Gloria Steinem",
        "\"The greatest glory in living lies not in never falling, but in "
        "rising every time we fall.\" - Nelson Mandela",
        "\"The future belongs to those who believe in the beauty of their "
        "dreams.\" - Eleanor Roosevelt",
        "\"The only limit to our realization of tomorrow will be our doubts of "
        "today.\" - Franklin D. Roosevelt",
        "\"The best way to predict the future is to create it.\" - Peter "
        "Drucker",
        "\"In the end, we will remember not the words of our enemies, but the "
        "silence of our friends.\" - Martin Luther King Jr.",
        "\"Either I will find a way, or I will make one.\" - Philip Sidney"};

    int index = std::rand() % static_cast<int>(quotes.size());
    return quotes[static_cast<std::vector<std::string>::size_type>(index)];
}

ChatBot getChatCmd(const std::string &command) {
    std::string uppercase_cmd = command;

    size_t first = uppercase_cmd.find_first_not_of(" \t\r\n\x01");
    if (std::string::npos == first) {
        uppercase_cmd = "";
    } else {
        size_t last = uppercase_cmd.find_last_not_of(" \t\r\n\x01");
        try {
            uppercase_cmd = uppercase_cmd.substr(first, (last - first + 1));
        } catch (const std::out_of_range &e) {
            std::cerr << "Failed to substr: " << e.what() << '\n';
            return ChatBot::UNKNOWN;
        }
    }

    std::transform(uppercase_cmd.begin(), uppercase_cmd.end(),
                   uppercase_cmd.begin(), ::toupper);
    static const std::unordered_map<std::string, ChatBot> commandMap = {
        {"HELLO", ChatBot::HELLO},       {"HI", ChatBot::HELLO},
        {"JOKE", ChatBot::JOKE},         {"HELP", ChatBot::HELP},
        {"QUOTE", ChatBot::QUOTE},       {"PING", ChatBot::PING},
        {"CHANNELS", ChatBot::CHANNELS}, {"WEATHER", ChatBot::WEATHER}};

    auto it = commandMap.find(uppercase_cmd);
    if (it == commandMap.end()) {
        return ChatBot::UNKNOWN;
    }
    return it->second;
}

ChatBot handleBotInput(const std::vector<std::string> &input) {
    if (input.empty()) {
        return (ChatBot::UNKNOWN);
    }

    const std::string &cmd = input[0];
    ChatBot action = getChatCmd(cmd);
    if (action == ChatBot::WEATHER) {
        if (input.size() == 1)
            return ChatBot::WEATHER_TOO_FEW;
        if (input.size() > 2)
            return ChatBot::WEATHER_TOO_MANY;
    }

    return action;
}
std::string getWeatherDirectly(const std::string &location,
                               const std::shared_ptr<Client> &client,
                               Server *server) {
    const char *hostname = "api.weatherapi.com";
    const char *port = "80";

    const char *api_key = std::getenv("MY_API_KEY");
    if (!api_key) {
        return "Error: API Key not configured.";
    }

    std::string full_path = "/v1/current.json?key=" + std::string(api_key) +
                            "&q=" + location + "&aqi=no";

    std::stringstream request_ss;
    request_ss << "GET " << full_path << " HTTP/1.1\r\n";
    request_ss << "Host: " << hostname << "\r\n";
    request_ss << "User-Agent: ft_irc_direct_http/0.1\r\n";
    request_ss << "Accept: application/json, */*\r\n";
    request_ss << "Connection: close\r\n";
    request_ss << "\r\n";

    int sockfd = getApiInfo(hostname, port);
    if (0 > sockfd) {
        return "Could not connect to API server.";
    }

    epoll_event ev{};
    ev.events = EPOLLOUT | EPOLLET;
    ev.data.fd = sockfd;
    epoll_ctl(server->getEpollFD(), EPOLL_CTL_ADD, sockfd, &ev);

    ApiRequest apiReq = {sockfd, client, "", request_ss.str(),
                         ApiRequest::CONNECTING};
    server->addApiRequest(apiReq);

    return "Fetching weather...";
}
} // namespace

std::string handleBot(const std::vector<std::string> &params,
                      const std::shared_ptr<Client> &client, Server *server) {
    std::string response;

    std::vector<std::string> input = split(params[1], " ");
    const ChatBot action = handleBotInput(input);
    switch (action) {
        case ChatBot::HELLO:
            response = "Hello " + client->getNickname() + "!";
            break;
        case ChatBot::WEATHER:
            response = getWeatherDirectly(input[1], client, server);
            break;
        case ChatBot::JOKE:
            response = getJoke();
            break;
        case ChatBot::PING:
            response = "PONG";
            break;
        case ChatBot::WEATHER_TOO_FEW:
            response = "Provide location for a weather.";
            break;
        case ChatBot::WEATHER_TOO_MANY:
            response = "I can check weather only for one location at time.";
            break;
        case ChatBot::QUOTE:
            response = getQuote();
            break;
        case ChatBot::HELP:
            response = "Supported commands: hello, weather, joke, "
                       "quote, ping, channels";
            break;
        case ChatBot::CHANNELS:
            response = server->getChannelsAndUsers();
            break;
        case ChatBot::UNKNOWN:
            response = "Command unknown. Type 'help' to discover my functions.";
            break;
    }

    return response;
}

bool isBot(const std::string &nickname) {
    std::string upperCase = nickname;
    std::transform(upperCase.begin(), upperCase.end(), upperCase.begin(),
                   ::toupper);

    if (upperCase == "BOT") {
        return (true);
    }
    return (false);
}

void botResponseNl(const std::shared_ptr<Client> &client,
                   const std::string &response) {
    std::string line;
    std::istringstream stream(response);

    while (std::getline(stream, line)) {
        handleMsg(IRCCode::PRIVMSG, client, ("Bot!Bot@codamirc.local"),
                  client->getNickname() + " :" + line);
    }
}

bool handleSendApi(ApiRequest &api, epoll_event event, int epoll_fd) {
    if (api.state == api.State::CONNECTING) {
        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(event.data.fd, SOL_SOCKET, SO_ERROR, &error, &len);

        if (error) {
            close(event.data.fd);
            return false;
        }

        send(event.data.fd, api.request.c_str(), api.request.size(),
             MSG_DONTWAIT | MSG_NOSIGNAL);
        api.state = ApiRequest::SENDING;

        epoll_event new_ev{};
        new_ev.events = EPOLLIN;
        new_ev.data.fd = event.data.fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &new_ev) == -1) {
            perror("epoll_ctl mod failed in handleSendApi");
            close(event.data.fd);
            api.fd = -1;
            botResponseNl(api.client,
                          "Server error setting up API response listener.");
            return false;
        }
    }

    return true;
}

void handleRecvApi(ApiRequest &api) {
    char buf[4096] = {0};
    ssize_t n = -1;
    bool connection_closed_by_peer = false;

    while (true) {
        n = recv(api.fd, buf, sizeof(buf), MSG_DONTWAIT);
        if (n > 0) {
            api.buffer.append(buf, static_cast<size_t>(n));
        } else if (n == 0) {
            connection_closed_by_peer = true;
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("recv error in handleRecvApi");
                botResponseNl(api.client, "Error receiving data from API.");
                if (api.fd != -1) {
                    close(api.fd);
                }
                api.fd = -1;
                return;
            }
        }
    }

    if (api.fd == -1) {
        return;
    }

    size_t header_end = api.buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        if (connection_closed_by_peer) {
            std::cerr
                << "handleRecvApi (fd=" << api.fd
                << "): Connection closed before finding headers. Buffer size: "
                << api.buffer.length() << '\n';
            botResponseNl(
                api.client,
                "Error: Incomplete response from API (connection closed).");
            if (api.fd != -1) {
                close(api.fd);
            }
            api.fd = -1;
            return;
        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return;
        } else {
            std::cerr << "handleRecvApi (fd=" << api.fd
                      << "): Headers not found, unexpected state." << '\n';
            botResponseNl(api.client,
                          "Error: Unexpected state receiving API response.");
            if (api.fd != -1) {
                close(api.fd);
            }
            api.fd = -1;
            return;
        }
    }

    try {
        std::string status_line = api.buffer.substr(0, api.buffer.find("\r\n"));
        if (status_line.find("200 OK") == std::string::npos) {
            std::cerr << "handleRecvApi (fd=" << api.fd
                      << "): API request failed. Status: " << status_line
                      << '\n';
            botResponseNl(api.client,
                          "API request failed. Status: " + status_line);
            if (api.fd != -1) {
                close(api.fd);
            }
            api.fd = -1;
            return;
        }

        std::string json_body = {};
        try {
            json_body = api.buffer.substr(header_end + 4);
        } catch (const std::out_of_range &e) {
            std::cerr << "Failed to substr: " << e.what() << '\n';
            close(api.fd);
            api.fd = -1;
            return;
        }

        if (json_body.empty()) {
            if (connection_closed_by_peer) {
                std::cerr << "handleRecvApi (fd=" << api.fd
                          << "): API response body is empty after headers "
                             "(connection closed)."
                          << '\n';
                botResponseNl(api.client,
                              "Error: Received empty response body from API.");
                if (api.fd != -1) {
                    close(api.fd);
                }
                api.fd = -1;
                return;
            } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                return;
            } else {
                std::cerr << "handleRecvApi (fd=" << api.fd
                          << "): API response body is empty (unknown reason)."
                          << '\n';
                botResponseNl(api.client,
                              "Error: Received empty response body from API.");
                if (api.fd != -1) {
                    close(api.fd);
                }
                api.fd = -1;
                return;
            }
        } else {
            std::string response = extractWeather(json_body);
            botResponseNl(api.client, response);
            if (api.fd != -1) {
                close(api.fd);
            }
            api.fd = -1;
            return;
        }
    } catch (const std::out_of_range &e) {
        std::cerr << "handleRecvApi (fd=" << api.fd
                  << "): Failed to extract/parse API response parts: "
                  << e.what() << '\n';
        botResponseNl(api.client, "Error processing API response structure.");
    } catch (const std::exception &e) {
        std::cerr << "handleRecvApi (fd=" << api.fd
                  << "): Error processing API response: " << e.what() << '\n';
        botResponseNl(api.client, "Error processing API response.");
    }

    if (api.fd != -1) {
        std::cerr << "handleRecvApi: Closing API socket fd " << api.fd
                  << " (reached end of function unexpectedly)" << '\n';
        close(api.fd);
        api.fd = -1;
    }
}
