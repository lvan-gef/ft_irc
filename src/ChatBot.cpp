#include <netdb.h> // For getaddrinfo, gai_strerror
#include <sstream> // For stringstream
#include <iomanip> // For std::setw with urlEncode
#include <string>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include <unistd.h>

#include "../include/ChatBot.hpp"

namespace {
std::string findVal(const std::string& json, const std::string& key) {
    std::string key_to_find = "\"" + key + "\":";
    size_t key_pos = json.find(key_to_find);

    if (key_pos == std::string::npos)
        return ("Key not found");

    size_t value_start = key_pos + key_to_find.length();

    while (value_start < json.length() && isspace(json[value_start])) {
        value_start++;
    }

    if (value_start >= json.length())
        return ("No value found after key");

    // Check if value is a string (starts with ")
    if (json[value_start] == '"') {
        size_t value_end = json.find('"', value_start + 1);
        // Basic check - doesn't handle escaped quotes \"
        if (value_end != std::string::npos) {
            return json.substr(value_start + 1, value_end - value_start - 1);
        }
    }
    // Check if value is a number (starts with digit or '-')
    else if (isdigit(json[value_start]) || json[value_start] == '-') {
        size_t value_end = value_start;
        while (value_end < json.length() && (isdigit(json[value_end]) || json[value_end] == '.' || json[value_end] == '-')) {
            value_end++;
        }
        return json.substr(value_start, value_end - value_start);
    }

    return ("Couldn't parse");
}

std::string extractWeather(const std::string& json) {
     if (json.empty() || json.rfind("Error:", 0) == 0) {
          return ("Json empty or error");
     }
     std::string temp_c = findVal(json, "temp_c");
     std::string description = findVal(json, "text"); 
     std::string location = findVal(json, "name");
    // std::string region = findVal(json, "region");
     std::string country = findVal(json, "country");


     if (temp_c.empty() || description.empty() || location.empty()) {
          return ("Error: Could not parse weather data from API response.");
     }

     std::string result = location;
    // if (!region.empty()) result += ", " + region;
     if (!country.empty()) result += ", " + country;
     result += ": " + temp_c + "°C, " + description;

     return (result);
}

}




ChatBot getChatCmd(const std::string &command) {
    std::string uppercase_cmd = command;

    size_t first = uppercase_cmd.find_first_not_of(" \t\r\n\x01");
    if (std::string::npos == first) {
        uppercase_cmd = "";
    } else {
        size_t last = uppercase_cmd.find_last_not_of(" \t\r\n\x01");
        uppercase_cmd = uppercase_cmd.substr(first, (last - first + 1));
    }

    std::transform(uppercase_cmd.begin(), uppercase_cmd.end(), uppercase_cmd.begin(), ::toupper);
    static const std::unordered_map<std::string, ChatBot> commandMap = {
        {"HELLO", ChatBot::HELLO},
        {"HI", ChatBot::HELLO},
		{"WEATHER", ChatBot::WEATHER},
        {"HELP", ChatBot::HELP},
		{"JOKE", ChatBot::JOKE},
		{"KICK", ChatBot::KICK},
		{"PING", ChatBot::PING},
		{"CHANNELS", ChatBot::CHANNELS}};
    auto it = commandMap.find(uppercase_cmd);
    if (it == commandMap.end()) {
        return (ChatBot::UNKNOWN);
    }
    return (it->second);
}

std::string getWeatherDirectly(const std::string& location) {
    const char* hostname = "api.weatherapi.com";
    const char* port = "80";

    const char* api_key_cstr = std::getenv("MY_API_KEY");
    if (!api_key_cstr)
         return ("Error: API Key not configured.");
    std::string api_key = api_key_cstr;

    struct addrinfo hints{}, *res = nullptr, *p = nullptr;
    int sockfd = -1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(hostname, port, &hints, &res);
    if (status != 0)
        return ("Could not resolve API hostname.");

    // --- 2. Create Socket and Connect ---
    for(p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            continue; // Try next address
        }

        // Optional: Set connect timeout here using non-blocking socket + select/poll if needed

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            sockfd = -1;
            continue; // Try next address
        }
        break; // Successfully connected
    }

    freeaddrinfo(res);

    if (sockfd == -1)
        return ("Could not connect to API server.");
    std::string full_path = "/v1/current.json?key=" + api_key + "&q=" + location + "&aqi=no";

    std::stringstream request_ss;
    request_ss << "GET " << full_path << " HTTP/1.1\r\n";
    request_ss << "Host: " << hostname << "\r\n";
    request_ss << "User-Agent: ft_irc_direct_http/0.1\r\n";
    request_ss << "Accept: application/json, */*\r\n"; // Be more specific
    request_ss << "Connection: close\r\n";
    request_ss << "\r\n";

    std::string request = request_ss.str();
    ssize_t sent = send(sockfd, request.c_str(), request.length(), 0);
    if (sent == -1) {
        close(sockfd);
        return ("Failed to send request.");
    }
    char buffer[4096];
    std::string response_str;
    ssize_t bytes_received;

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        response_str += buffer;
    }

    if (bytes_received == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
             close(sockfd);
             return ("Timed out receiving response from API.");
        } else {
             close(sockfd);
             return ("Failed to receive response from API.");
        }
    }

    close(sockfd);

    size_t header_end = response_str.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return ("Invalid HTTP response (no header end).");
    }

    std::string status_line = response_str.substr(0, response_str.find("\r\n"));
    if (status_line.find("200 OK") == std::string::npos) {
         // Check for redirects (3xx) which are common from HTTP to HTTPS
         if (status_line.find(" 30") != std::string::npos) { // Basic check for 301, 302 etc.
             return ("API requires HTTPS, cannot connect via HTTP.");
         }
         return ("API request failed (Status: " + status_line + ")");
    }

	response_str = extractWeather(response_str);
    return response_str;
}

ChatBot handleBotInput(std::vector<std::string> input)
{
	if (input.empty())
		return (ChatBot::UNKNOWN);
	
	std::string cmd = input[0];
	ChatBot action = getChatCmd(cmd);
	if (action == ChatBot::WEATHER)
	{
		if (input.size() == 1)
			return (ChatBot::WEATHER_TOO_FEW);
		if (input.size() > 2)
			return (ChatBot::WEATHER_TOO_MANY);
	}
	return (action);
}

namespace {

std::string getJoke(void)
{
	return ("So far it works like that");
}


}

std::string handleBot(std::vector<std::string> params, const std::shared_ptr<Client> &client,
						Server *server)
{
	std::string response;
	
	std::vector<std::string> input = split(params[1], " ");
	const ChatBot action = handleBotInput(input);
	switch(action)
	{
		case ChatBot::HELLO:
			response = "Hello " + client->getNickname() + "!";
			break ;
		case ChatBot::WEATHER:
			response = getWeatherDirectly(input[1]);
			break ;
		case ChatBot::PING:
			response = "PONG";
			break ;
		case ChatBot::WEATHER_TOO_FEW:
			response = "Provide location for a weather.";
			break ;
		case ChatBot::WEATHER_TOO_MANY:
			response = "I can check weather only for one location at time.";
			break ;
		case ChatBot::JOKE:
			response  = getJoke();
			break ;
		case ChatBot::KICK:
			response = "KICK";
			break ;
		case ChatBot::HELP:
			response = "Hello, I am bot. I can say hello, check weather and send PONG back.";
			break ;
		case ChatBot::CHANNELS:
			response = server->getChannelsAndUsers();
			break ;
		default:
			response = "Command unknown. Type 'help' to discover my functions.";
			break ;
			//add another cases with weather
	}
	return (response);
}

bool	isBot(std::string nickname)
{
	std::string upperCase = nickname;
	std::transform(upperCase.begin(), upperCase.end(), upperCase.begin(), ::toupper);
	std::cout << "Nick after transformation >>>>>" << upperCase << "<<<<<<" << std::endl;
	std::cout << "Len: 3?" << upperCase.length() << std::endl;
	if (upperCase == "BOT")
	{
		std::cout << "YUP ITS BOT" << std::endl;
		return (true);
	}
	std::cout << "NOPE NOT BOT 0O" << std::endl;
	return (false);
}