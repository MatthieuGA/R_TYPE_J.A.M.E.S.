#pragma once
#include <utility>
#include <vector>

#include <boost/asio.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <server/Config.hpp>

namespace server {
class Network {
 public:
    static constexpr int QUEUE_SIZE = 4096;
    static constexpr int MAX_UDP_PACKET_SIZE = 1472;

    Network() = delete;
    Network(const Network &) = delete;
    Network &operator=(const Network &) = delete;
    Network(Network &&) = delete;
    Network &operator=(Network &&) = delete;
    ~Network() = default;

    explicit Network(Config &, boost::asio::io_context &);

    struct PlayerInput {
        uint8_t id;
        uint8_t inputState;
    };

    std::optional<PlayerInput> QueuePop() {
        PlayerInput input;
        if (udp_.queue.pop(input)) {
            return input;
        }
        return std::nullopt;
    }

 private:
    class UDP {
     public:
        using ReceiveCallback =
            std::function<void(const boost::asio::ip::udp::endpoint &,
                const std::vector<uint8_t> &)>;

        explicit UDP(Config &config, boost::asio::io_context &io);
        void Send(const std::array<uint8_t, MAX_UDP_PACKET_SIZE> &data,
            std::size_t size, const boost::asio::ip::udp::endpoint &endpoint);
        void Receive();
        boost::asio::ip::port_type Port() const;

        void SetReceiveCallback(ReceiveCallback callback) {
            on_receive_ = std::move(callback);
        }

        boost::lockfree::spsc_queue<PlayerInput,
            boost::lockfree::capacity<QUEUE_SIZE>>
            queue;

     private:
        boost::asio::ip::udp::socket socket;
        std::array<uint8_t, MAX_UDP_PACKET_SIZE> buffer;
        boost::asio::ip::udp::endpoint remote_endpoint;
        ReceiveCallback on_receive_;
    };

    class TCP {
     public:
        using AcceptCallback =
            std::function<void(boost::asio::ip::tcp::socket)>;

        explicit TCP(Config &config, boost::asio::io_context &io);
        void Accept();
        boost::asio::ip::port_type Port() const;

        /**
         * @brief Set callback for new TCP connections
         *
         * When a new connection is accepted, socket ownership is transferred
         * to the callback (typically Server::HandleTcpAccept).
         *
         * @param callback Function to invoke with new socket
         */
        void SetAcceptCallback(AcceptCallback callback) {
            on_accept_ = std::move(callback);
        }

        boost::asio::ip::tcp::acceptor acceptor_;

     private:
        AcceptCallback on_accept_;
    };

    UDP udp_;
    TCP tcp_;

 public:
    /**
     * @brief Get reference to TCP handler for callback registration
     *
     * @return TCP& Reference to internal TCP handler
     */
    TCP &GetTcp() {
        return tcp_;
    }

    /**
     * @brief Get UDP server port
     *
     * @return uint16_t UDP port number
     */
    uint16_t GetUdpPort() const {
        return udp_.Port();
    }

    /**
     * @brief Set callback for UDP receives
     *
     * @param callback Function to invoke with received endpoint and data
     */
    void SetUdpReceiveCallback(UDP::ReceiveCallback callback) {
        udp_.SetReceiveCallback(std::move(callback));
    }

    /**
     * @brief Send UDP packet to a specific endpoint
     *
     * @param data Packet data to send
     * @param size Number of bytes to send
     * @param endpoint UDP endpoint to send to
     */
    void SendUdp(const std::array<uint8_t, MAX_UDP_PACKET_SIZE> &data,
        std::size_t size, const boost::asio::ip::udp::endpoint &endpoint);
};
}  // namespace server
