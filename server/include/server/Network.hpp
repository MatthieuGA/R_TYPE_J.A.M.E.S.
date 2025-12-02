#pragma once
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

 private:
    class UDP {
     public:
        explicit UDP(Config &config, boost::asio::io_context &io);
        void send(const std::array<uint8_t, MAX_UDP_PACKET_SIZE> &data,
            std::size_t size, const boost::asio::ip::udp::endpoint &endpoint);
        void receive();
        boost::asio::ip::port_type port() const;

     private:
        boost::asio::ip::udp::socket udpSocket;
        std::array<uint8_t, MAX_UDP_PACKET_SIZE> udpBuffer;
        boost::asio::ip::udp::endpoint udpRemoteEndpoint;
        boost::lockfree::spsc_queue<PlayerInput,
            boost::lockfree::capacity<QUEUE_SIZE>>
            udpQueue;
    };

    class TCP {
     public:
        explicit TCP(Config &config, boost::asio::io_context &io);
        void accept();
        boost::asio::ip::port_type port() const;

        boost::asio::ip::tcp::acceptor tcpAcceptor;

     private:
    };

    UDP _udp;
    TCP _tcp;
};
}  // namespace server
