#pragma once
#include <boost/asio.hpp>
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

 private:
    // SPSC queue for UDP packets, size QUEUE_SIZE
    boost::asio::ip::tcp::acceptor tcpAcceptor;
    boost::asio::ip::udp::socket udpSocket;
    std::array<uint8_t, MAX_UDP_PACKET_SIZE> udpBuffer;
    boost::asio::ip::udp::endpoint udpRemoteEndpoint;

    void do_tcp_accept();   // Accept new TCP connections handler
    void do_udp_receive();  // Receive UDP packets handler
};
}  // namespace server
