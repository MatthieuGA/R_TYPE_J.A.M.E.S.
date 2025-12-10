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
    /**
     * @brief Get the singleton Config instance initialized from command line
     * arguments
     *
     * @param argc Argument count
     * @param argv Argument values
     * @return Config& Reference to the singleton configuration
     */
    static Config &FromCommandLine(int argc, char *argv[]);

    /**
     * @brief Parse command line arguments into a new Config object
     *
     * Useful for testing without modifying the singleton state.
     *
     * @param argc Argument count
     * @param argv Argument values
     * @return Config Parsed configuration object
     */
    static Config Parse(int argc, char *argv[]);

    /**
     * @brief Get the configured TCP port
     * @return uint16_t TCP port number
     */
    uint16_t GetTcpPort() const {
        return tcpPort_;
    }

    /**
     * @brief Get the configured UDP port
     * @return uint16_t UDP port number
     */
    uint16_t GetUdpPort() const {
        return udpPort_;
    }

    /**
     * @brief Get the maximum number of allowed players
     * @return uint8_t Max players (1-255)
     */
    uint8_t GetMaxPlayers() const {
        return maxPlayers_;
    }

    /**
     * @brief Get the TCP bind address
     * @return const std::string& IP address string
     */
    const std::string &GetTcpAddress() const {
        return tcpAddress_;
    }

    /**
     * @brief Get the UDP bind address
     * @return const std::string& IP address string
     */
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
