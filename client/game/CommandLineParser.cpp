#include "game/CommandLineParser.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Rtype::Client {

namespace {
constexpr uint16_t kDefaultPort = 50000;
}  // namespace

void CommandLineParser::PrintUsageAndExit(
    const char *program_name, const char *error_message) {
    if (error_message) {
        std::cerr << "Error: " << error_message << "\n\n";
    }
    std::cerr << "Usage: " << program_name
              << " <USERNAME> [IP] [TCP-PORT] [-up/--udp-port UDP-PORT]\n"
              << "\n"
              << "Positional arguments:\n"
              << "  USERNAME     Player username (max 32 characters)\n"
              << "  IP           Server IP address (default: 127.0.0.1, solo "
                 "mode)\n"
              << "  TCP-PORT     TCP port number (1-65535, default: 50000)\n"
              << "\n"
              << "Optional arguments:\n"
              << "  -up, --udp-port UDP-PORT\n"
              << "               UDP port number (1-65535).\n"
              << "               Defaults to TCP-PORT if not specified.\n"
              << "\n"
              << "Modes:\n"
              << "  Solo mode:   Only USERNAME provided. Spawns a local "
                 "server.\n"
              << "  Online mode: IP and TCP-PORT provided. Connects to remote "
                 "server.\n"
              << "\n"
              << "Examples:\n"
              << "  " << program_name << " Player1\n"
              << "  " << program_name << " Player1 192.168.1.100\n"
              << "  " << program_name << " Player1 192.168.1.100 50000\n"
              << "  " << program_name
              << " Player1 192.168.1.100 50000 --udp-port 50001\n";
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
    config.solo_mode = false;

    // Check for help flag
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help" || arg == "-?" || arg == "help") {
            PrintUsageAndExit(argv[0]);
        }
    }

    // Count positional arguments (non-flag arguments)
    std::vector<std::string> positional_args;
    std::vector<std::pair<std::string, int>> flags;  // flag name and index

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-up" || arg == "--udp-port") {
            if (i + 1 >= argc) {
                PrintUsageAndExit(
                    argv[0], "Missing value for -up/--udp-port flag");
            }
            flags.emplace_back(arg, i + 1);
            ++i;  // Skip the value
        } else if (arg[0] == '-') {
            std::string error = "Unknown argument: " + arg;
            PrintUsageAndExit(argv[0], error.c_str());
        } else {
            positional_args.push_back(arg);
        }
    }

    // Require at least 1 positional argument: USERNAME
    if (positional_args.empty()) {
        PrintUsageAndExit(argv[0], "Missing required argument: USERNAME");
    }

    // Parse based on number of positional arguments
    // New syntax: <USERNAME> [IP] [TCP-PORT]
    config.username = positional_args[0];

    // Validate username
    if (config.username.empty()) {
        PrintUsageAndExit(argv[0], "USERNAME cannot be empty");
    }
    if (config.username.length() > 32) {
        PrintUsageAndExit(argv[0], "USERNAME too long (max 32 characters)");
    }

    if (positional_args.size() == 1) {
        // Solo mode: only username provided
        config.solo_mode = true;
        config.server_ip = "127.0.0.1";
        config.tcp_port = kDefaultPort;  // Will be updated by ServerSpawner
    } else if (positional_args.size() == 2) {
        // IP provided, use default port
        config.solo_mode = false;
        config.server_ip = positional_args[1];
        config.tcp_port = kDefaultPort;
    } else if (positional_args.size() >= 3) {
        // Full explicit mode: USERNAME IP TCP-PORT
        config.solo_mode = false;
        config.server_ip = positional_args[1];
        try {
            config.tcp_port = ParsePort(positional_args[2], "TCP-PORT");
        } catch (const std::exception &e) {
            PrintUsageAndExit(argv[0], e.what());
        }

        // Check for unexpected extra positional arguments
        if (positional_args.size() > 3) {
            std::string error = "Unexpected argument: " + positional_args[3];
            PrintUsageAndExit(argv[0], error.c_str());
        }
    }

    // Default UDP port to TCP port
    config.udp_port = config.tcp_port;

    // Parse optional UDP port flag
    for (const auto &[_, value_index] : flags) {
        try {
            config.udp_port = ParsePort(argv[value_index], "UDP-PORT");
        } catch (const std::exception &e) {
            PrintUsageAndExit(argv[0], e.what());
        }
    }

    return config;
}

}  // namespace Rtype::Client
