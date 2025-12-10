#include <iostream>
#include <stdexcept>
#include <string>

#include <server/Config.hpp>

namespace server {
int Config::StringToPort(const char *str) {
    try {
        return std::stoi(str);
    } catch (const std::exception &e) {
        throw std::invalid_argument("Port must be a number");
    }
}

void Config::PrintUsage(const char *programName) {
    std::cout << "Usage: " << programName
              << " [TCP_PORT] [UDP_PORT] [-p MAX_PLAYERS]" << std::endl;
    std::cout << "  TCP_PORT: Port for TCP connections (default: "
              << DEFAULT_TCP_PORT << ")" << std::endl;
    std::cout
        << "  UDP_PORT: Port for UDP connections (default: same as TCP port)"
        << std::endl;
    std::cout << "  -p MAX_PLAYERS: Maximum number of players (default: "
              << static_cast<int>(DEFAULT_MAX_PLAYERS) << ", max: 255)"
              << std::endl;
}

void Config::PrintHelp(const char *programName) {
    PrintUsage(programName);
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " 50000 50001 -p 8" << std::endl;
    std::cout << "This starts the R-Type server with TCP port 50000, UDP "
                 "port 50001, and max 8 players."
              << std::endl;
}

void Config::ParseMaxPlayers(int &i, int argc, char *argv[], Config &config) {
    if (i + 1 >= argc) {
        throw std::invalid_argument("Missing value for -p");
    }
    try {
        int max_players = std::stoi(argv[++i]);
        if (max_players < 1 || max_players > 255) {
            throw std::out_of_range("Max players must be between 1 and 255");
        }
        config.maxPlayers_ = static_cast<uint8_t>(max_players);
    } catch (const std::exception &e) {
        std::cerr << "Invalid max players: " << argv[i] << std::endl;
        throw;
    }
}

void Config::ParsePositionalArgs(
    const std::string &arg, int &positional_index, Config &config) {
    if (positional_index == 0) {
        try {
            int tcp_port_value = StringToPort(arg.c_str());
            if (tcp_port_value < 1 || tcp_port_value > 65535) {
                throw std::out_of_range("Port must be between 1 and 65535");
            }
            config.tcpPort_ = static_cast<uint16_t>(tcp_port_value);
        } catch (const std::exception &e) {
            std::cerr << "Invalid TCP port: " << arg << std::endl;
            throw;
        }
        positional_index++;
    } else if (positional_index == 1) {
        try {
            int udp_port_value = StringToPort(arg.c_str());
            if (udp_port_value < 1 || udp_port_value > 65535) {
                throw std::out_of_range("Port must be between 1 and 65535");
            }
            config.udpPort_ = static_cast<uint16_t>(udp_port_value);
        } catch (const std::exception &e) {
            std::cerr << "Invalid UDP port: " << arg << std::endl;
            throw;
        }
        positional_index++;
    } else {
        // We don't have argv[0] here, so we can't print usage with program
        // name But Parse() handles the exception and can print usage if
        // needed, or we just throw.
        throw std::invalid_argument("Too many arguments provided");
    }
}

Config Config::Parse(int argc, char *argv[]) {
    Config config;
    int positional_index = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            PrintHelp(argv[0]);
            std::exit(0);
        } else if (arg == "-p") {
            ParseMaxPlayers(i, argc, argv, config);
        } else {
            try {
                ParsePositionalArgs(arg, positional_index, config);
            } catch (const std::invalid_argument &e) {
                if (std::string(e.what()) == "Too many arguments provided") {
                    PrintUsage(argv[0]);
                }
                throw;
            }
        }
    }

    // If UDP port was not specified, default to TCP port
    if (positional_index == 1) {
        config.udpPort_ = config.tcpPort_;
    }

    return config;
}

Config &Config::FromCommandLine(int argc, char *argv[]) {
    static Config config = Parse(argc, argv);
    return config;
}
}  // namespace server
