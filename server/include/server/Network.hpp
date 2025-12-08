#pragma once
#include <utility>

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
        uint8_t playerId;
        uint8_t inputState;
    };

    std::optional<PlayerInput> queuePop() {
        PlayerInput input;
        if (_udp.queue.pop(input)) {
            return input;
        }
        return std::nullopt;
    }

 private:
    class UDP {
     public:
        explicit UDP(Config &config, boost::asio::io_context &io);
        void send(const std::array<uint8_t, MAX_UDP_PACKET_SIZE> &data,
            std::size_t size, const boost::asio::ip::udp::endpoint &endpoint);
        void receive();
        boost::asio::ip::port_type port() const;

        boost::lockfree::spsc_queue<PlayerInput,
            boost::lockfree::capacity<QUEUE_SIZE>>
            queue;

     private:
        boost::asio::ip::udp::socket socket;
        std::array<uint8_t, MAX_UDP_PACKET_SIZE> buffer;
        boost::asio::ip::udp::endpoint remote_endpoint;
    };

    class TCP {
     public:
        using AcceptCallback =
            std::function<void(boost::asio::ip::tcp::socket)>;

        explicit TCP(Config &config, boost::asio::io_context &io);
        void accept();
        boost::asio::ip::port_type port() const;

        /**
         * @brief Set callback for new TCP connections
         *
         * When a new connection is accepted, socket ownership is transferred
         * to the callback (typically Server::HandleTcpAccept).
         *
         * @param callback Function to invoke with new socket
         */
        void setAcceptCallback(AcceptCallback callback) {
            on_accept_ = std::move(callback);
        }

        boost::asio::ip::tcp::acceptor acceptor;

     private:
        AcceptCallback on_accept_;
    };

    UDP _udp;
    TCP _tcp;

 public:
    /**
     * @brief Get reference to TCP handler for callback registration
     *
     * @return TCP& Reference to internal TCP handler
     */
    TCP &getTcp() {
        return _tcp;
    }
};
}  // namespace server
