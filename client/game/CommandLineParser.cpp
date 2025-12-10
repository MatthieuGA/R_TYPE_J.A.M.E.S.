#include "game/CommandLineParser.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace Rtype::Client {

void CommandLineParser::PrintUsageAndExit(
    const char *program_name, const char *error_message) {
    if (error_message) {
        std::cerr << "Error: " << error_message << "\n\n";
    }
    std::cerr << "Usage: " << program_name
              << " <IP> <TCP-PORT> <USERNAME> [-up/--udp-port UDP-PORT]\n"
              << "\n"
              << "Positional arguments:\n"
              << "  IP           Server IP address (e.g., 127.0.0.1)\n"
              << "  TCP-PORT     TCP port number (1-65535)\n"
              << "  USERNAME     Player username (max 32 characters)\n"
              << "\n"
              << "Optional arguments:\n"
              << "  -up, --udp-port UDP-PORT\n"
              << "               UDP port number (1-65535).\n"
              << "               Defaults to TCP-PORT if not specified.\n"
              << "\n"
              << "Examples:\n"
              << "  " << program_name << " 127.0.0.1 50000 Test\n"
              << "  " << program_name << " 127.0.0.1 50000 Test -up 50001\n"
              << "  " << program_name
              << " 192.168.1.100 50000 Player1 --udp-port 50001\n";
    std::exit(error_message ? EXIT_FAILURE : EXIT_SUCCESS);
}

uint16_t CommandLineParser::ParsePort(
    const std::string &port_str, const char *port_name) {
    try {
        int port = std::stoi(port_str);
        if (port < 1 || port > 65535) {
            std::string error =
                std::string(port_name) + " must be between 1 and 65535";
            throw std::out_of_range(error);
        }
        return static_cast<uint16_t>(port);
    } catch (const std::exception &e) {
        std::string error = "Invalid " + std::string(port_name) + ": " +
                            port_str + " (" + e.what() + ")";
        throw std::runtime_error(error);
    }
}

ClientConfig CommandLineParser::Parse(int argc, char *argv[]) {
    ClientConfig config;

    // Check for help flag
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help" || arg == "-?" || arg == "help") {
            PrintUsageAndExit(argv[0]);
        }
    }

    // Require at least 3 positional arguments: IP, TCP-PORT, USERNAME
    if (argc < 4) {
        PrintUsageAndExit(argv[0],
            "Missing required arguments. Expected at least 3 arguments.");
    }

    // Parse positional arguments
    config.server_ip = argv[1];
    try {
        config.tcp_port = ParsePort(argv[2], "TCP-PORT");
    } catch (const std::exception &e) {
        PrintUsageAndExit(argv[0], e.what());
    }
    config.username = argv[3];

    // Validate username length
    if (config.username.empty()) {
        PrintUsageAndExit(argv[0], "USERNAME cannot be empty");
    }
    if (config.username.length() > 32) {
        PrintUsageAndExit(argv[0], "USERNAME too long (max 32 characters)");
    }

    // Default UDP port to TCP port
    config.udp_port = config.tcp_port;

    // Parse optional UDP port flag
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-up" || arg == "--udp-port") {
            if (i + 1 >= argc) {
                PrintUsageAndExit(
                    argv[0], "Missing value for -up/--udp-port flag");
            }
            try {
                config.udp_port = ParsePort(argv[i + 1], "UDP-PORT");
            } catch (const std::exception &e) {
                PrintUsageAndExit(argv[0], e.what());
            }
            ++i;  // Skip the port value in next iteration
        } else {
            std::string error = "Unknown argument: " + arg;
            PrintUsageAndExit(argv[0], error.c_str());
        }
    }

    return config;
}

}  // namespace Rtype::Client
