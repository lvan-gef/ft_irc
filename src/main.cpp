#include <iostream>

#include "../include/Server.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Expect 2 arguments: port and password, got: " << argc - 1
                  << '\n';
        return 1;
    }

    try {
        Server server(argv[1], argv[2]);
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 2;
    }

    return 0;
}
