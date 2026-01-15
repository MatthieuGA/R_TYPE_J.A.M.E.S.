#pragma once
#include <cstdint>
#include <string>

namespace Rtype::Client {

/**
 * @brief Configuration structure for client network parameters.
 */
struct ClientConfig {
    std::string server_ip;
    uint16_t tcp_port;
    uint16_t udp_port;
    std::string username;
    bool solo_mode;  ///< True if client should spawn its own local server
    std::string graphics_backend;  // Graphics backend name (e.g., "sfml")
                                   // Empty string means use default
};

/**
 * @brief Command-line argument parser for R-Type client.
 */
class CommandLineParser {
 public:
    /**
     * @brief Parse command-line arguments into ClientConfig.
     *
     * Parses arguments with the following syntax:
     * <IP> <TCP-PORT> <USERNAME> [-up/--udp-port UDP-PORT]
     * [--graphics-backend=<NAME>]
     *
     * Optional flags:
     *   -up, --udp-port <PORT>     Specify UDP port (default from config or
     * command)
     *   --graphics-backend=<NAME>  Specify graphics backend name (e.g.,
     * "sfml")
     *
     * @param argc Argument count
     * @param argv Argument values
     * @return ClientConfig Parsed configuration
     * @throws std::runtime_error if arguments are invalid
     */
    static ClientConfig Parse(int argc, char *argv[]);

 private:
    /**
     * @brief Print usage information and exit.
     *
     * @param program_name The name of the executable (argv[0])
     * @param error_message Optional error message to display
     */
    static void PrintUsageAndExit(
        const char *program_name, const char *error_message = nullptr);

    /**
     * @brief Parse and validate a port number.
     *
     * @param port_str Port number as string
     * @param port_name Name of the port (for error messages)
     * @return uint16_t Validated port number
     * @throws std::runtime_error if port is invalid
     */
    static uint16_t ParsePort(
        const std::string &port_str, const char *port_name);
};

}  // namespace Rtype::Client
