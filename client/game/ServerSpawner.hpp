#pragma once
#include <cstdint>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
/* Forward-declare HANDLE to avoid pulling full Windows headers into the public
   header. The implementation file includes <windows.h> where needed. */
typedef void *HANDLE;
#else
#include <sys/types.h>
#endif

namespace Rtype::Client {

/**
 * @brief Utility class for spawning and managing a local server process.
 *
 * Used in solo mode to automatically start a server when the client
 * launches without explicit server connection parameters.
 */
class ServerSpawner {
 public:
    /// Starting port for auto-detection
    static constexpr uint16_t kStartPort = 50000;
    /// Maximum port to try before failing
    static constexpr uint16_t kMaxPort = 50100;

    /**
     * @brief Spawn a local server process.
     *
     * Finds an available port (starting from kStartPort), spawns the server
     * executable, and returns the selected port.
     *
     * @return uint16_t The port the server is listening on
     * @throws std::runtime_error if no port is available or spawn fails
     */
    static uint16_t SpawnLocalServer();

    /**
     * @brief Terminate the spawned server process.
     *
     * Sends termination signal and waits for the process to exit.
     * Safe to call even if no server was spawned.
     */
    static void TerminateServer();

    /**
     * @brief Check if a server process is currently running.
     *
     * @return true if a server was spawned and is still running
     */
    static bool IsServerRunning();

    /**
     * @brief Get the port the local server is listening on.
     *
     * @return uint16_t The selected port, or 0 if no server is running
     */
    static uint16_t GetServerPort();

    /**
     * @brief Setup signal handlers for graceful cleanup.
     *
     * Installs handlers for SIGINT/SIGTERM that will terminate the
     * spawned server before exiting.
     */
    static void SetupSignalHandlers();

 private:
#ifdef _WIN32
    static HANDLE server_process_;
#else
    static pid_t server_pid_;
#endif
    static uint16_t server_port_;
    static bool server_running_;

    /**
     * @brief Check if a TCP port is available for binding.
     *
     * @param port Port number to check
     * @return true if the port is available
     */
    static bool IsPortAvailable(uint16_t port);

    /**
     * @brief Find the server executable path.
     *
     * Looks for r-type_server in common locations relative to the client.
     *
     * @return std::string Path to the server executable
     * @throws std::runtime_error if server executable not found
     */
    static std::string FindServerExecutable();
};

/**
 * @brief RAII wrapper to ensure server cleanup on scope exit.
 *
 * Terminates the spawned server when the guard goes out of scope,
 * ensuring cleanup on normal exit, exceptions, or errors.
 */
class ServerGuard {
 public:
    /**
     * @brief Construct a ServerGuard.
     * @param solo_mode If true, will terminate server on destruction
     */
    explicit ServerGuard(bool solo_mode) : solo_mode_(solo_mode) {}

    ~ServerGuard() {
        if (solo_mode_) {
            ServerSpawner::TerminateServer();
        }
    }

    // Non-copyable
    ServerGuard(const ServerGuard &) = delete;
    ServerGuard &operator=(const ServerGuard &) = delete;

 private:
    bool solo_mode_;
};

}  // namespace Rtype::Client
