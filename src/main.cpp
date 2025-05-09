#include <iostream>
#include <string>
#include <exception>

#include "../include/Server.hpp"

int main(const int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Expect 2 arguments: port and password, got: " << argc - 1
                  << '\n';
        return 1;
    }

    try {
        const std::string arg1 = argv[1];
        std::string arg2 = argv[2];
        Server server = Server(arg1, arg2);

        if (server.init() != true) {
            return 2;
        }

        server.run();
    } catch (const std::exception &e) {
        std::cerr << "Server error: " << e.what() << '\n';
        return 3;
    }

    std::cout << "Server is succesfully stopped" << '\n';
    return 0;
}
