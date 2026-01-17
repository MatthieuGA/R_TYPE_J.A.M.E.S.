#include "game/ServerSpawner.hpp"

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace Rtype::Client {

// Static member initialization
#ifdef _WIN32
HANDLE ServerSpawner::server_process_ = nullptr;
#else
pid_t ServerSpawner::server_pid_ = -1;
#endif
uint16_t ServerSpawner::server_port_ = 0;
bool ServerSpawner::server_running_ = false;

namespace {

// Mutex for thread-safe access to static variables
std::mutex g_server_mutex;

#ifdef _WIN32
// Cached Winsock initialization state (call WSAStartup once, not per port)
class WinsockInitializer {
 public:
    WinsockInitializer() {
        WSADATA wsa_data;
        initialized_ = (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0);
    }

    ~WinsockInitializer() {
        if (initialized_) {
            WSACleanup();
        }
    }

    bool IsInitialized() const {
        return initialized_;
    }

 private:
    bool initialized_ = false;
};

WinsockInitializer &GetWinsock() {
    static WinsockInitializer instance;
    return instance;
}
#endif

}  // namespace

bool ServerSpawner::IsPortAvailable(uint16_t port) {
#ifdef _WIN32
    if (!GetWinsock().IsInitialized()) {
        return false;
    }
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return false;
    }
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return false;
    }
#endif

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int result =
        bind(sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));

#ifdef _WIN32
    closesocket(sock);
    return result != SOCKET_ERROR;
#else
    close(sock);
    return result == 0;
#endif
}

std::string ServerSpawner::FindServerExecutable() {
    namespace fs = std::filesystem;

    // Try common locations relative to executable
    std::vector<std::string> search_paths = {
        "./r-type_server",
        "../server/r-type_server",
        "../r-type_server",
        "build/server/r-type_server",
        "./build/server/r-type_server",
#ifdef _WIN32
        "./r-type_server.exe",
        "../server/r-type_server.exe",
        "../r-type_server.exe",
        "build/server/r-type_server.exe",
        "./build/server/r-type_server.exe",
#endif
    };

    for (const auto &path : search_paths) {
        if (fs::exists(path)) {
            return fs::absolute(path).string();
        }
    }

    throw std::runtime_error(
        "Could not find r-type_server executable. "
        "Make sure it is built and in a standard location.");
}

uint16_t ServerSpawner::SpawnLocalServer() {
    std::lock_guard<std::mutex> lock(g_server_mutex);

    if (server_running_) {
        return server_port_;
    }

    // Find available port
    uint16_t selected_port = 0;
    for (uint16_t port = kStartPort; port <= kMaxPort; ++port) {
        if (IsPortAvailable(port)) {
            selected_port = port;
            break;
        }
    }

    if (selected_port == 0) {
        throw std::runtime_error(
            "No available port found in range " + std::to_string(kStartPort) +
            "-" + std::to_string(kMaxPort) +
            ". Please close other applications using these ports.");
    }

    std::string server_path = FindServerExecutable();
    std::string port_str = std::to_string(selected_port);

    std::cout << "[Solo Mode] Starting local server on port " << selected_port
              << "..." << std::endl;

#ifdef _WIN32
    // Windows: Use CreateProcess
    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    std::string cmd_line = server_path + " " + port_str;

    // CreateProcessA may modify the command line buffer, so use mutable copy
    std::vector<char> cmd_buffer(cmd_line.begin(), cmd_line.end());
    cmd_buffer.push_back('\0');

    if (!CreateProcessA(nullptr, cmd_buffer.data(), nullptr, nullptr, FALSE,
            CREATE_NEW_CONSOLE,  // Create new console window
            nullptr, nullptr, &si, &pi)) {
        throw std::runtime_error(
            "Failed to start server process. Error code: " +
            std::to_string(GetLastError()));
    }

    server_process_ = pi.hProcess;
    CloseHandle(pi.hThread);
#else
    // Linux/Unix: Use fork + exec
    pid_t pid = fork();

    if (pid < 0) {
        throw std::runtime_error("Failed to fork server process");
    }

    if (pid == 0) {
        // Child process: exec the server
        execl(server_path.c_str(), "r-type_server", port_str.c_str(), nullptr);
        // If exec fails, exit child
        std::cerr << "[Solo Mode] Failed to exec server: " << server_path
                  << std::endl;
        _exit(EXIT_FAILURE);
    }

    // Parent process
    server_pid_ = pid;
#endif

    server_port_ = selected_port;
    server_running_ = true;

    // Give the server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "[Solo Mode] Server started successfully (PID: "
#ifdef _WIN32
              << GetProcessId(server_process_)
#else
              << server_pid_
#endif
              << ")" << std::endl;

    return selected_port;
}

void ServerSpawner::TerminateServer() {
    std::lock_guard<std::mutex> lock(g_server_mutex);

    if (!server_running_) {
        return;
    }

    std::cout << "[Solo Mode] Shutting down local server..." << std::endl;

#ifdef _WIN32
    if (server_process_ != nullptr) {
        TerminateProcess(server_process_, 0);
        WaitForSingleObject(server_process_, 3000);  // Wait up to 3 seconds
        CloseHandle(server_process_);
        server_process_ = nullptr;
    }
#else
    if (server_pid_ > 0) {
        // Send SIGTERM for graceful shutdown
        kill(server_pid_, SIGTERM);

        // Wait for child to exit (with timeout)
        int status;
        for (int i = 0; i < 30; ++i) {  // Wait up to 3 seconds
            pid_t result = waitpid(server_pid_, &status, WNOHANG);
            if (result > 0) {
                break;  // Child exited
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Force kill if still running
        if (waitpid(server_pid_, &status, WNOHANG) == 0) {
            kill(server_pid_, SIGKILL);
            waitpid(server_pid_, &status, 0);
        }

        server_pid_ = -1;
    }
#endif

    server_running_ = false;
    server_port_ = 0;

    std::cout << "[Solo Mode] Server stopped." << std::endl;
}

bool ServerSpawner::IsServerRunning() {
    if (!server_running_) {
        return false;
    }

#ifdef _WIN32
    if (server_process_ != nullptr) {
        DWORD exit_code;
        if (GetExitCodeProcess(server_process_, &exit_code)) {
            return exit_code == STILL_ACTIVE;
        }
    }
    return false;
#else
    if (server_pid_ > 0) {
        int status;
        pid_t result = waitpid(server_pid_, &status, WNOHANG);
        if (result == 0) {
            return true;  // Still running
        }
        // Process has exited
        server_running_ = false;
        server_pid_ = -1;
    }
    return false;
#endif
}

uint16_t ServerSpawner::GetServerPort() {
    return server_port_;
}

namespace {

#ifndef _WIN32
/**
 * @brief Signal handler callback for graceful shutdown (POSIX).
 *
 * This handler only performs async-signal-safe operations.
 * It writes a static message to standard error and then
 * terminates the process using _Exit.
 *
 * @param signal The received POSIX signal.
 */
void SignalHandlerCallback(int signal) {
#ifdef SIGINT
    if (signal == SIGINT) {
        static const char kMsgInt[] =
            "\n[Client] Received SIGINT, shutting down...\n";
        ::write(STDERR_FILENO, kMsgInt, sizeof(kMsgInt) - 1);
        ::_Exit(EXIT_SUCCESS);
    }
#endif
#ifdef SIGTERM
    if (signal == SIGTERM) {
        static const char kMsgTerm[] =
            "\n[Client] Received SIGTERM, shutting down...\n";
        ::write(STDERR_FILENO, kMsgTerm, sizeof(kMsgTerm) - 1);
        ::_Exit(EXIT_FAILURE);
    }
#endif
    static const char kMsgOther[] =
        "\n[Client] Received termination signal, shutting down...\n";
    ::write(STDERR_FILENO, kMsgOther, sizeof(kMsgOther) - 1);
    ::_Exit(EXIT_FAILURE);
}
#endif

}  // namespace

#ifdef _WIN32
/**
 * @brief Windows console control handler for graceful shutdown.
 *
 * Called when the user presses Ctrl+C or closes the console window.
 *
 * @param ctrl_type The type of console control event.
 * @return TRUE if handled, FALSE to pass to next handler.
 */
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrl_type) {
    switch (ctrl_type) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
            ServerSpawner::TerminateServer();
            return TRUE;
        default:
            return FALSE;
    }
}
#endif

void ServerSpawner::SetupSignalHandlers() {
#ifdef _WIN32
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
#else
    std::signal(SIGINT, SignalHandlerCallback);
    std::signal(SIGTERM, SignalHandlerCallback);
#endif
}

}  // namespace Rtype::Client
