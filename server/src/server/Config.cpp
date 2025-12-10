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
    std::cout << "Usage: " << programName << " [TCP_PORT] [UDP_PORT]"
              << std::endl;
    std::cout << "  TCP_PORT: Port for TCP connections (default: "
              << DEFAULT_TCP_PORT << ")" << std::endl;
    std::cout
        << "  UDP_PORT: Port for UDP connections (default: same as TCP port)"
        << std::endl;
}

void Config::PrintHelp(const char *programName) {
    PrintUsage(programName);
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " 50000 50001" << std::endl;
    std::cout << "This starts the R-Type server with TCP port 50000 and UDP "
                 "port 50001."
              << std::endl;
}

Config &Config::FromCommandLine(int argc, char *argv[]) {
    static Config config;
    static bool initialized = false;

    // Ensure the configuration is only initialized once
    if (initialized) {
        return config;
    }

    // Handle help arguments
    if (argc >= 2 &&
        (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        PrintHelp(argv[0]);
        std::exit(0);
    }

    // Parse TCP port (optional, defaults to DEFAULT_TCP_PORT)
    if (argc >= 2) {
        try {
            int tcpPortValue = StringToPort(argv[1]);
            if (tcpPortValue < 1 || tcpPortValue > 65535) {
                throw std::out_of_range("Port must be between 1 and 65535");
            }
            config.tcpPort_ = static_cast<uint16_t>(tcpPortValue);
        } catch (const std::exception &e) {
            std::cerr << "Invalid TCP port: " << argv[1] << std::endl;
            throw;
        }
    }

    // Parse UDP port (optional, defaults to same as TCP port)
    if (argc >= 3) {
        try {
            int udpPortValue = StringToPort(argv[2]);
            if (udpPortValue < 1 || udpPortValue > 65535) {
                throw std::out_of_range("Port must be between 1 and 65535");
            }
            config.udpPort_ = static_cast<uint16_t>(udpPortValue);
        } catch (const std::exception &e) {
            std::cerr << "Invalid UDP port: " << argv[2] << std::endl;
            throw;
        }
    } else {
        config.udpPort_ = config.tcpPort_;
    }

    // Error message if too many arguments
    if (argc > 3) {
        PrintUsage(argv[0]);
        throw std::invalid_argument("Too many arguments provided");
    }
    initialized = true;
    return config;
}
}  // namespace server
