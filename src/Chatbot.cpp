#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <unordered_map>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#include "../include/Chatbot.hpp"
#include "../include/Enums.hpp"
#include "../include/Utils.hpp"
#include "Server.hpp"

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
    struct addrinfo hints{}, *res = nullptr, *p = nullptr;
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

std::string readApiResponse(int sockfd) {
    char buffer[4096];
    std::string response_str;
    ssize_t bytes_received = 0;

    struct timeval tv{};
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response_str += buffer;
    }

    if (bytes_received == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            close(sockfd);
            return "Timed out receiving response from API.";
        } else {
            close(sockfd);
            return "Failed to receive response from API.";
        }
    }
    close(sockfd);

    return response_str;
}

std::string getQuote() {
    const char *hostname = "api.quotable.io";
    const char *port = "80";

    int sockfd = getApiInfo(hostname, port);
    if (sockfd == -1) {
        return ("Could not connect to API server.");
    }

    std::stringstream request_ss;
    request_ss << "GET /quotes/random" << " HTTP/1.1\r\n";
    request_ss << "Host: " << hostname << "\r\n";
    request_ss << "User-Agent: ft_irc_direct_http/0.1\r\n";
    request_ss << "Accept: application/json, */*\r\n";
    request_ss << "Connection: close\r\n";
    request_ss << "\r\n";

    std::string request = request_ss.str();
    ssize_t sent = send(sockfd, request.c_str(), request.length(), 0);
    if (sent == -1) {
        close(sockfd);
        return ("Failed to send request.");
    }

    const std::string response_str = readApiResponse(sockfd);
    size_t header_end = response_str.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return "Invalid HTTP response (no header end).";
    }

    std::string json_body;
    try {
        json_body = response_str.substr(header_end + 4);
    } catch (const std::out_of_range &e) {
        std::cerr << "Failed to substr: " << e.what() << '\n';
        return "Internal server error";
    }

    std::string content = findVal(json_body, "content");
    std::string author = findVal(json_body, "author");
    if (content == "Key not found" || author == "Key not found") {
        return "Error: Could not parse quote data from API response.";
    }
    return "\"" + content + "\" - " + author;
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
        {"WEATHER", ChatBot::WEATHER},   {"HELP", ChatBot::HELP},
        {"QUOTE", ChatBot::QUOTE},       {"PING", ChatBot::PING},
        {"CHANNELS", ChatBot::CHANNELS},
    };

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

    return (action);
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

    // Store request context
    ApiRequest apiReq = {sockfd, client, "", request_ss.str(),
                         ApiRequest::CONNECTING};
    server->addApiRequest(apiReq);

    return "Fetching weather..."; // Immediate ack

    // std::string request = request_ss.str();
    // ssize_t sent = send(sockfd, request.c_str(), request.length(), 0);
    // if (sent == -1) {
    //     close(sockfd);
    //     return "Failed to send request.";
    // }
    //
    // std::string response_str = readApiResponse(sockfd);
    // size_t header_end = response_str.find("\r\n\r\n");
    // if (header_end == std::string::npos) {
    //     return "Invalid HTTP response (no header end).";
    // }
    //
    // std::string status_line;
    // try {
    //     status_line = response_str.substr(0, response_str.find("\r\n"));
    // } catch (const std::out_of_range &e) {
    //     std::cerr << "Failed to substr: " << e.what() << '\n';
    //     return "Internal server error";
    // }
    //
    // if (status_line.find("200 OK") == std::string::npos) {
    //     if (status_line.find(" 30") != std::string::npos) {
    //         return "API requires HTTPS, cannot connect via HTTP.";
    //     }
    //     return "API request failed (Status: " + status_line + ")";
    // }
    //
    // return extractWeather(response_str);
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
            response = "Supported commands: hello, weather <location>, joke, "
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

        send(event.data.fd, api.request.c_str(), api.request.size(), 0);
        api.state = ApiRequest::SENDING;

        //epoll_event new_ev{};
        //new_ev.events = EPOLLIN | EPOLLET;
        //new_ev.data.fd = event.data.fd;
        //epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &new_ev);
		epoll_event new_ev{};
        new_ev.events = EPOLLIN; // Just EPOLLIN (implies level-triggered)
        new_ev.data.fd = event.data.fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &new_ev) == -1) {
             perror("epoll_ctl mod failed in handleSendApi");
             // Handle error: close socket, notify client, mark api.fd = -1
             close(event.data.fd);
             api.fd = -1;
             botResponseNl(api.client, "Server error setting up API response listener.");
             return false; // Indicate failure
        }
        std::cout << "handleSendApi: Request sent, modified epoll for EPOLLIN (LT) on fd " << event.data.fd << std::endl;
    }

    return true;
}

//void handleRecvApi(ApiRequest &api) {
//    char buf[4096];

//    ssize_t n = recv(api.fd, buf, sizeof(buf), 0);
//    while (n > 0) {
//        api.buffer.append(buf, static_cast<size_t>(n));
//        n = recv(api.fd, buf, sizeof(buf), 0);
//    }
//	if (errno != EWOULDBLOCK)
//	{
//		//std::cout << "Would block" << std::endl;
//		std::string response = extractWeather(api.buffer);
//		std::cout << response << '\n';
//		botResponseNl(api.client, response);
//		close(api.fd);
//	}
//}


// ...existing code...

// REPLACE the existing handleRecvApi function with this one:
void handleRecvApi(ApiRequest &api) {
    char buf[4096];
    ssize_t n;
    bool connection_closed_by_peer = false;

    // Loop to read all available data (works for LT and is necessary for ET)
    while (true) {
        // Socket should be non-blocking (set in getApiInfo)
        n = recv(api.fd, buf, sizeof(buf), 0);
        if (n > 0) {
            // Append received data to the buffer
            api.buffer.append(buf, static_cast<size_t>(n));
        } else if (n == 0) {
            // Connection closed by peer, finished reading
            connection_closed_by_peer = true;
            std::cout << "handleRecvApi: Connection closed by peer (fd=" << api.fd << ")." << std::endl;
            break;
        } else { // n == -1
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data available to read *right now*
                // For LT: epoll will notify again if more data arrives.
                // For ET: We assume we've read everything for this event cycle.
                std::cout << "handleRecvApi: recv returned EAGAIN/EWOULDBLOCK (fd=" << api.fd << ")." << std::endl;
                break; // Exit the read loop
            } else {
                // An actual read error occurred
                perror("recv error in handleRecvApi");
                botResponseNl(api.client, "Error receiving data from API.");
                if (api.fd != -1) close(api.fd);
                api.fd = -1; // Mark as closed/error
                // NOTE: The server loop MUST remove this api request from the map
                return; // Exit the function
            }
        }
    }

    // Check if the socket is still valid (wasn't closed due to error)
    // ... (read loop remains the same) ...

    // Check if the socket is still valid (wasn't closed due to error in read loop)
    if (api.fd == -1) {
        return; // Already handled error and closed socket
    }

    std::cout << "handleRecvApi: Finished reading loop (fd=" << api.fd << "). Total buffer length: " << api.buffer.length() << std::endl;

    // --- Processing the accumulated buffer ---

    // Find the end of the HTTP headers (\r\n\r\n)
    size_t header_end = api.buffer.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        // Headers not found yet.
        if (connection_closed_by_peer) {
             // Connection closed before headers arrived. Error.
             std::cerr << "handleRecvApi (fd=" << api.fd << "): Connection closed before finding headers. Buffer size: " << api.buffer.length() << std::endl;
             botResponseNl(api.client, "Error: Incomplete response from API (connection closed).");
             // Close and mark for removal
             if (api.fd != -1) close(api.fd);
             api.fd = -1;
             // NOTE: Server loop MUST remove request
             return;
        } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
             // Hit EAGAIN, but haven't found headers yet.
             // Using LT: Just return and wait for more data.
             std::cout << "handleRecvApi (fd=" << api.fd << "): Headers not found yet, waiting for more data (EAGAIN)." << std::endl;
             return; // <<<<<<<<<<<< RETURN WITHOUT CLOSING
        } else {
             // Should not happen?
             std::cerr << "handleRecvApi (fd=" << api.fd << "): Headers not found, unexpected state." << std::endl;
             botResponseNl(api.client, "Error: Unexpected state receiving API response.");
             if (api.fd != -1) close(api.fd);
             api.fd = -1;
             // NOTE: Server loop MUST remove request
             return;
        }
    }

    // Headers found, proceed to parse body
    std::cout << "handleRecvApi (fd=" << api.fd << "): Found header end at position " << header_end << std::endl;
    std::string json_body;
    try {
        // Check HTTP status code
        std::string status_line = api.buffer.substr(0, api.buffer.find("\r\n"));
        if (status_line.find("200 OK") == std::string::npos) {
             std::cerr << "handleRecvApi (fd=" << api.fd << "): API request failed. Status: " << status_line << std::endl;
             botResponseNl(api.client, "API request failed. Status: " + status_line);
             // Close and mark for removal as the request failed
             if (api.fd != -1) close(api.fd);
             api.fd = -1;
             // NOTE: Server loop MUST remove request
             return;
        }

        // Status is OK, try to extract body
        json_body = api.buffer.substr(header_end + 4); // +4 for \r\n\r\n
        std::cout << "handleRecvApi (fd=" << api.fd << "): Extracted potential JSON body length: " << json_body.length() << std::endl;

        if (json_body.empty()) {
            // Body is empty.
            if (connection_closed_by_peer) {
                 // Connection closed and body is empty. Might be valid (e.g., 204) or error.
                 std::cerr << "handleRecvApi (fd=" << api.fd << "): API response body is empty after headers (connection closed)." << std::endl;
                 botResponseNl(api.client, "Error: Received empty response body from API.");
                 // Close and mark for removal
                 if (api.fd != -1) close(api.fd);
                 api.fd = -1;
                 // NOTE: Server loop MUST remove request
                 return;
            } else if (n == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                 // Hit EAGAIN, body is empty. Headers are present.
                 // Using LT: Just return and wait for body data.
                 std::cout << "handleRecvApi (fd=" << api.fd << "): Body not found yet, waiting for more data (EAGAIN)." << std::endl;
                 return; // <<<<<<<<<<<< RETURN WITHOUT CLOSING
            } else {
                 // Unknown reason for empty body
                 std::cerr << "handleRecvApi (fd=" << api.fd << "): API response body is empty (unknown reason)." << std::endl;
                 botResponseNl(api.client, "Error: Received empty response body from API.");
                 if (api.fd != -1) close(api.fd);
                 api.fd = -1;
                 // NOTE: Server loop MUST remove request
                 return;
            }
        } else {
            // Body has content, parse it
            std::string response = extractWeather(json_body); // Pass ONLY the body
            std::cout << "Parsed API RESPONSE (fd=" << api.fd << "): " << response << '\n';
            botResponseNl(api.client, response);
            // Request successful and processed, close and mark for removal
            if (api.fd != -1) close(api.fd);
            api.fd = -1;
            // NOTE: Server loop MUST remove request
            return;
        }
    } catch (const std::out_of_range &e) {
         std::cerr << "handleRecvApi (fd=" << api.fd << "): Failed to extract/parse API response parts: " << e.what() << '\n';
         botResponseNl(api.client, "Error processing API response structure.");
    } catch (const std::exception& e) { // Catch other potential errors
         std::cerr << "handleRecvApi (fd=" << api.fd << "): Error processing API response: " << e.what() << '\n';
         botResponseNl(api.client, "Error processing API response.");
    }

    // If we reach here due to an exception or logic error before returning, ensure cleanup
    if (api.fd != -1) {
        std::cout << "handleRecvApi: Closing API socket fd " << api.fd << " (reached end of function unexpectedly)" << std::endl;
        close(api.fd);
        api.fd = -1; // Mark as closed
    }
    // NOTE: The server loop MUST remove this api request from the _api_requests map
    //       because api.fd is now -1.
}


// ...existing code...