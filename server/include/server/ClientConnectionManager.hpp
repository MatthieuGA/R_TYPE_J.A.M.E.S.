#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

#include <boost/asio.hpp>

namespace server {

/**
 * @brief Represents an active client connection to the server
 *
 * This structure encapsulates all state associated with a connected client,
 * including both TCP (reliable) and UDP (unreliable) endpoints, player
 * identification, and connection metadata.
 */
struct ClientConnection {
    uint32_t client_id_;  // Internal unique ID (never sent over network)
    uint8_t player_id_;   // Network player ID (1-255, 0 if not authenticated)
    mutable boost::asio::ip::tcp::socket
        tcp_socket_;  // Mutable to allow async operations on const references
    boost::asio::ip::udp::endpoint udp_endpoint_;
    std::chrono::steady_clock::time_point last_activity_;
    std::string username_;
    bool ready_;  // For lobby ready state

    /**
     * @brief Construct a ClientConnection with moved socket
     *
     * @param cid Unique internal client ID
     * @param pid Assigned player ID (1-255, 0 if not yet authenticated)
     * @param socket TCP socket (ownership transferred)
     */
    ClientConnection(
        uint32_t cid, uint8_t pid, boost::asio::ip::tcp::socket socket)
        : client_id_(cid),
          player_id_(pid),
          tcp_socket_(std::move(socket)),
          udp_endpoint_(boost::asio::ip::udp::endpoint()),
          last_activity_(std::chrono::steady_clock::now()),
          ready_(false) {
        username_.reserve(32);  // Match CONNECT_REQ username size
    }

    /**
     * @brief Check if client is authenticated
     *
     * @return true if player_id is assigned (non-zero)
     */
    bool IsAuthenticated() const {
        return player_id_ != 0;
    }

    // Disable copy (socket is not copyable)
    ClientConnection(const ClientConnection &) = delete;
    ClientConnection &operator=(const ClientConnection &) = delete;

    // Enable move
    ClientConnection(ClientConnection &&) = default;
    ClientConnection &operator=(ClientConnection &&) = default;
};

/**
 * @brief Manages all active client connections
 *
 * Responsible for:
 * - Storing and owning ClientConnection objects (including sockets)
 * - Assigning unique client_id and player_id values
 * - Validating usernames and enforcing max player limits
 * - Providing access to connections via GetClient()
 */
class ClientConnectionManager {
 public:
    /**
     * @brief Construct a new ClientConnectionManager
     *
     * @param max_clients Maximum number of authenticated players allowed
     */
    explicit ClientConnectionManager(uint8_t max_clients);

    /**
     * @brief Add a new unauthenticated client connection
     *
     * Assigns a unique client_id and stores the connection.
     * Client starts with player_id=0 (unauthenticated).
     *
     * @param socket TCP socket (ownership transferred)
     * @return uint32_t Assigned client_id
     */
    uint32_t AddClient(boost::asio::ip::tcp::socket socket);

    /**
     * @brief Authenticate a client by assigning a player_id and username
     *
     * Validates username availability and server capacity.
     * Returns 0 if authentication fails (username taken or server full).
     *
     * @param client_id Internal client ID
     * @param username Requested username
     * @return uint8_t Assigned player_id (1-255), or 0 on failure
     */
    uint8_t AuthenticateClient(
        uint32_t client_id, const std::string &username);

    /**
     * @brief Remove a client connection
     *
     * Closes TCP socket and removes from active connections.
     *
     * @param client_id Internal client ID to remove
     */
    void RemoveClient(uint32_t client_id);

    /**
     * @brief Get a client connection by ID
     *
     * @param client_id Internal client ID
     * @return ClientConnection& Reference to the connection
     * @throws std::out_of_range if client_id not found
     */
    ClientConnection &GetClient(uint32_t client_id);

    /**
     * @brief Get a client connection by ID (const version)
     *
     * @param client_id Internal client ID
     * @return const ClientConnection& Reference to the connection
     * @throws std::out_of_range if client_id not found
     */
    const ClientConnection &GetClient(uint32_t client_id) const;

    /**
     * @brief Check if a client exists
     *
     * @param client_id Internal client ID
     * @return true if client exists
     */
    bool HasClient(uint32_t client_id) const;

    /**
     * @brief Check if username is already taken
     *
     * @param username Username to check
     * @return true if username exists among authenticated clients
     */
    bool IsUsernameTaken(const std::string &username) const;

    /**
     * @brief Get count of authenticated players
     *
     * @return size_t Number of clients with player_id != 0
     */
    size_t GetAuthenticatedCount() const;

    /**
     * @brief Check if server is full
     *
     * @return true if authenticated player count >= max_clients_
     */
    bool IsFull() const;

    /**
     * @brief Get maximum number of clients allowed
     *
     * @return uint8_t Maximum number of authenticated players
     */
    uint8_t GetMaxClients() const;

    /**
     * @brief Check if all authenticated players are ready
     *
     * @return true if all authenticated clients have ready_ == true
     */
    bool AllPlayersReady() const;

    /**
     * @brief Get all client connections (for iteration)
     *
     * @return const std::unordered_map<uint32_t, ClientConnection>&
     */
    const std::unordered_map<uint32_t, ClientConnection> &GetClients() const;

 private:
    /**
     * @brief Assign unique internal client ID
     *
     * @return uint32_t Unique client ID (never reused)
     */
    uint32_t AssignClientId();

    /**
     * @brief Assign unique PlayerId to authenticated client
     *
     * Searches for available player_id in range [1, 255].
     *
     * @return uint8_t PlayerId in range [1, 255]
     * @throws std::runtime_error if all player IDs exhausted
     */
    uint8_t AssignPlayerId();

    // All client connections mapped by internal client_id
    std::unordered_map<uint32_t, ClientConnection> clients_;

    // Next available client ID (internal, never sent over network)
    uint32_t next_client_id_;

    // Next available PlayerId (1-255, sent in packets)
    uint8_t next_player_id_;

    // Maximum number of authenticated players allowed
    uint8_t max_clients_;
};

}  // namespace server
