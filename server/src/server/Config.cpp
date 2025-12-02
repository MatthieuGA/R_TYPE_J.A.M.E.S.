#include <iostream>
#include <stdexcept>
#include <string>

#include <server/Config.hpp>

namespace server {
int Config::stringToPort(const char *str) {
    try {
        return std::stoi(str);
    } catch (const std::exception &e) {
        throw std::invalid_argument("Port must be a number");
    }
}

void Config::printUsage(const char *programName) {
    std::cout << "Usage: " << programName << " [TCP_PORT] [UDP_PORT]"
              << std::endl;
    std::cout << "  TCP_PORT: Port for TCP connections (default: "
              << DEFAULT_TCP_PORT << ")" << std::endl;
    std::cout
        << "  UDP_PORT: Port for UDP connections (default: same as TCP port)"
        << std::endl;
}

void Config::printHelp(const char *programName) {
    printUsage(programName);
    std::cout << "Example:" << std::endl;
    std::cout << "  " << programName << " 50000 50001" << std::endl;
    std::cout << "This starts the R-Type server with TCP port 50000 and UDP "
                 "port 50001."
              << std::endl;
}

Config &Config::fromCommandLine(int argc, char *argv[]) {
    static Config config;
    static bool initialized = false;

    // Ensure the configuration is only initialized once
    if (initialized) {
        return config;
    }

    // Handle help arguments
    if (argc >= 2 &&
        (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        printHelp(argv[0]);
        std::exit(0);
    }

    // Parse TCP port (optional, defaults to DEFAULT_TCP_PORT)
    if (argc >= 2) {
        try {
            int tcpPortValue = stringToPort(argv[1]);
            if (tcpPortValue < 1 || tcpPortValue > 65535) {
                throw std::out_of_range("Port must be between 1 and 65535");
            }
            config.tcpPort = static_cast<uint16_t>(tcpPortValue);
        } catch (const std::exception &e) {
            std::cerr << "Invalid TCP port: " << argv[1] << std::endl;
            throw;
        }
    }

    // Parse UDP port (optional, defaults to same as TCP port)
    if (argc >= 3) {
        try {
            int udpPortValue = stringToPort(argv[2]);
            if (udpPortValue < 1 || udpPortValue > 65535) {
                throw std::out_of_range("Port must be between 1 and 65535");
            }
            config.udpPort = static_cast<uint16_t>(udpPortValue);
        } catch (const std::exception &e) {
            std::cerr << "Invalid UDP port: " << argv[2] << std::endl;
            throw;
        }
    } else {
        config.udpPort = config.tcpPort;
    }

    // Error message if too many arguments
    if (argc > 3) {
        printUsage(argv[0]);
        throw std::invalid_argument("Too many arguments provided");
    }
    initialized = true;
    return config;
}
}  // namespace server
