#pragma once
#include <cstdint>
#include <string>

namespace server {
constexpr uint16_t DEFAULT_TCP_PORT = 50000;
constexpr uint16_t DEFAULT_UDP_PORT = DEFAULT_TCP_PORT;
constexpr const char *DEFAULT_TCP_ADDRESS = "0.0.0.0";
constexpr const char *DEFAULT_UDP_ADDRESS = DEFAULT_TCP_ADDRESS;
constexpr uint8_t DEFAULT_MAX_PLAYERS = 4;

class Config {
 public:
    static Config &FromCommandLine(int argc, char *argv[]);

    static Config Parse(int argc, char *argv[]);

    uint16_t GetTcpPort() const {
        return tcpPort_;
    }

    uint16_t GetUdpPort() const {
        return udpPort_;
    }

    const std::string &GetTcpAddress() const {
        return tcpAddress_;
    }

    const std::string &GetUdpAddress() const {
        return udpAddress_;
    }

 private:
    std::string tcpAddress_ = DEFAULT_TCP_ADDRESS;
    uint16_t tcpPort_ = DEFAULT_TCP_PORT;
    std::string udpAddress_ =
        DEFAULT_UDP_ADDRESS;               // Default is same as TCP address
    uint16_t udpPort_ = DEFAULT_UDP_PORT;  // Default is same as TCP port
    uint8_t maxPlayers_ = DEFAULT_MAX_PLAYERS;

    Config() = default;
    static int StringToPort(const char *str);
    static void PrintUsage(const char *programName);
    static void PrintHelp(const char *programName);
};
}  // namespace server
