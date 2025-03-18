/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: lvan-gef <lvan-gef@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2025/02/28 16:45:59 by lvan-gef      #+#    #+#                 */
/*   Updated: 2025/02/28 16:46:01 by lvan-gef      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <string>

#include "../include/Server.hpp"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Expect 2 arguments: port and password, got: " << argc - 1
                  << '\n';
        return 1;
    }

    std::string arg1 = argv[1];
    std::string arg2 = argv[2];
    try {
        Server server(arg1, arg2);

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
