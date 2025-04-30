#include <algorithm>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <unordered_map>

#include "../include/Chatbot.hpp"
#include "../include/Enums.hpp"
#include "../include/Utils.hpp"

namespace {

std::string getJoke()
{
    static const std::vector<std::string> jokes = {
        "Why did the scarecrow win an award? Because he was outstanding in his field!",
        "Algorithm: A word used by programmers when they don't want to explain how their code works.",
        "Debugging is like being the detective in a crime movie where you're also the murderer at the same time.",
        "The six stages of debugging:\n1. That can't happen.\n2. That doesn't happen on my machine.\n3." \
        " That shouldn't happen.\n4. Why does that happen?\n5. Oh, I see.\n6. How did that ever work?",
        "My husband and I were happy for 20 years. And then we met.",
        "What kind of doctor is Dr. Pepper?, \nHe's a fizzician.",
        "Why did the coffee file a police report? \nIt got mugged.",
        "What do you call a fish wearing a bowtie? \nSofishticated.",
        "I told my suitcase we're not going on vacation — now it's dealing with emotional baggage."        
    };
    
    
    int index = std::rand() % static_cast<int> (jokes.size());
    return jokes[static_cast<std::vector<std::string>::size_type>(index)];
}

std::string getQuote() {

    static const std::vector<std::string> quotes ={
        "\"Your friend is your needs answered.\" - Kahlil Gibran",
        "\"How is it possible to find meaning in a finite world, given my waist and shirt size?\" - Woody Allen",
        "\"A true friend freely, advises justly, assists readily, adventures boldly, takes all patiently," \
        " defends courageously, and continues a friend unchangeably.\" - William C. Menninger",
        "\"Imagination is the highest kite one can fly.\" - Lauren Bacall",
        "\"The personal life deeply lived always expands into truths beyond itself.\" - Anaïs Nin",
        "\"Without leaps of imagination, or dreaming, we lose the excitement of possibilities. Dreaming, after all, is a form of planning.\" - Gloria Steinem",
        "\"The greatest glory in living lies not in never falling, but in rising every time we fall.\" - Nelson Mandela",
        "\"The future belongs to those who believe in the beauty of their dreams.\" - Eleanor Roosevelt",
        "\"The only limit to our realization of tomorrow will be our doubts of today.\" - Franklin D. Roosevelt",
        "\"The best way to predict the future is to create it.\" - Peter Drucker",
        "\"In the end, we will remember not the words of our enemies, but the silence of our friends.\" - Martin Luther King Jr.",
        "\"Either I will find a way, or I will make one.\" - Philip Sidney"
    };

    int index = std::rand() % static_cast<int> (quotes.size());
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

    return (action);
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
        case ChatBot::JOKE:
            response = getJoke();
            break;
        case ChatBot::PING:
            response = "PONG";
            break;
        case ChatBot::QUOTE:
            response = getQuote();
            break;
        case ChatBot::HELP:
            response = "Supported commands: hello, joke, "
                       "quote, ping, channels";
            break;
        case ChatBot::CHANNELS:
            response = server->getChannelsAndUsers();
            break;
        case ChatBot::UNKNOWN:
            response = "Command unknown. Type 'help' to discover my functions.";
            break;
    }
    return (response);
}

bool isBot(const std::string &nickname) {
    std::string upperCase = nickname;
    std::transform(upperCase.begin(), upperCase.end(), upperCase.begin(),
                   ::toupper);

    if (upperCase == "BOT") {
        ;
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
