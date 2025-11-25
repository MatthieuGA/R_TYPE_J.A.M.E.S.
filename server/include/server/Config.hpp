#pragma once
#include <cstdint>
#include <string>

namespace server {
constexpr uint16_t DEFAULT_TCP_PORT = 50000;
constexpr uint16_t DEFAULT_UDP_PORT = DEFAULT_TCP_PORT;

class Config {
 public:
    static Config &fromCommandLine(int argc, char *argv[]);

 private:
    std::string tcpAddress = "0.0.0.0";
    uint16_t tcpPort = DEFAULT_TCP_PORT;
    std::string udpAddress = "0.0.0.0";   // Default is same as TCP address
    uint16_t udpPort = DEFAULT_UDP_PORT;  // Default is same as TCP port

    Config() = default;
    static int stringToPort(const char *str);
    static void printUsage(const char *programName);
    static void printHelp(const char *programName);
};
}  // namespace server
