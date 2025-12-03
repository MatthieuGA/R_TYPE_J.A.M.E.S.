#pragma once
#include <cstdint>
#include <string>

namespace server {
constexpr uint16_t DEFAULT_TCP_PORT = 50000;
constexpr uint16_t DEFAULT_UDP_PORT = DEFAULT_TCP_PORT;
constexpr const char *DEFAULT_TCP_ADDRESS = "0.0.0.0";
constexpr const char *DEFAULT_UDP_ADDRESS = DEFAULT_TCP_ADDRESS;

class Config {
 public:
    static Config &fromCommandLine(int argc, char *argv[]);

    uint16_t getTcpPort() const {
        return tcpPort;
    }

    uint16_t getUdpPort() const {
        return udpPort;
    }

    const std::string &getTcpAddress() const {
        return tcpAddress;
    }

    const std::string &getUdpAddress() const {
        return udpAddress;
    }

 private:
    std::string tcpAddress = DEFAULT_TCP_ADDRESS;
    uint16_t tcpPort = DEFAULT_TCP_PORT;
    std::string udpAddress =
        DEFAULT_UDP_ADDRESS;              // Default is same as TCP address
    uint16_t udpPort = DEFAULT_UDP_PORT;  // Default is same as TCP port

    Config() = default;
    static int stringToPort(const char *str);
    static void printUsage(const char *programName);
    static void printHelp(const char *programName);
};
}  // namespace server
