#include <iostream>
#include <utility>
#include <vector>

#include <server/Network.hpp>

namespace server {
Network::Network(Config &config, boost::asio::io_context &io)
    : tcp_(config, io), udp_(config, io) {
    std::cout << "Waiting for TCP connections on port " << tcp_.Port() << "..."
              << std::endl;
    std::cout << "Waiting for UDP packets on port " << udp_.Port() << "..."
              << std::endl;
    tcp_.Accept();
    udp_.Receive();
}

Network::UDP::UDP(Config &config, boost::asio::io_context &io)
    : socket(io, boost::asio::ip::udp::endpoint(
                     boost::asio::ip::make_address(config.GetUdpAddress()),
                     config.GetUdpPort())),
      on_receive_(nullptr) {}

Network::TCP::TCP(Config &config, boost::asio::io_context &io)
    : acceptor_(io, boost::asio::ip::tcp::endpoint(
                        boost::asio::ip::make_address(config.GetTcpAddress()),
                        config.GetTcpPort())) {}

boost::asio::ip::port_type Network::UDP::Port() const {
    return socket.local_endpoint().port();
}

boost::asio::ip::port_type Network::TCP::Port() const {
    return acceptor_.local_endpoint().port();
}

void Network::TCP::Accept() {
    try {
        acceptor_.async_accept([this](boost::system::error_code ec,
                                   boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                std::cout << "New TCP connection from "
                          << socket.remote_endpoint().address().to_string()
                          << ":" << socket.remote_endpoint().port()
                          << std::endl;

                // Transfer socket ownership to Server class via callback
                if (on_accept_) {
                    on_accept_(std::move(socket));
                } else {
                    std::cerr << "Warning: No TCP accept callback registered, "
                              << "closing connection" << std::endl;
                }
            } else {
                std::cerr << "Error accepting TCP connection: " << ec.message()
                          << std::endl;
            }
            Accept();  // Accept next connection (async chain)
        });
    } catch (const std::exception &e) {
        std::cerr << "Exception in async_accept: " << e.what() << std::endl;
    }
}

void Network::UDP::Receive() {
    try {
        socket.async_receive_from(boost::asio::buffer(buffer), remote_endpoint,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::cout << "Received UDP packet from "
                              << remote_endpoint.address().to_string() << ":"
                              << remote_endpoint.port()
                              << ", size: " << bytes_recvd << " bytes"
                              << std::endl;

                    // Call receive callback to update client endpoint
                    if (on_receive_) {
                        std::vector<uint8_t> data(
                            buffer.begin(), buffer.begin() + bytes_recvd);
                        on_receive_(remote_endpoint, data);
                    }
                } else if (ec && ec.value() != 10054 && ec.value() != 10064 &&
                           ec.value() != 10061) {
                    // Suppress WSAECONNRESET (10054),
                    //     WSAENETRESET (10064), WSAECONNREFUSED (10061)
                    // which are expected when no one is listening or
                    //     client hasn't bound UDP yet
                    std::cerr << "Error receiving UDP packet: " << ec.message()
                              << std::endl;
                }
                Receive();  // Receive next UDP packet (async chain)
            });
    } catch (const std::exception &e) {
        std::cerr << "Exception in async_receive_from: " << e.what()
                  << std::endl;
    }
}

void Network::UDP::Send(
    const std::array<uint8_t, Network::MAX_UDP_PACKET_SIZE> &data,
    std::size_t size, const boost::asio::ip::udp::endpoint &endpoint) {
    try {
        socket.async_send_to(boost::asio::buffer(data, size), endpoint,
            [endpoint](boost::system::error_code ec, std::size_t bytes_sent) {
                if (ec) {
                    // ICMP "connection refused" (10061) is normal when client
                    // hasn't started listening yet - suppress verbose logging
                    if (ec.value() != 10061) {
                        std::cerr << "Error sending UDP packet to "
                                  << endpoint.address().to_string() << ":"
                                  << endpoint.port() << ": " << ec.message()
                                  << std::endl;
                    }
                } else {
                    std::cout << "Sent UDP packet to "
                              << endpoint.address().to_string() << ":"
                              << endpoint.port() << ": " << bytes_sent
                              << " bytes" << std::endl;
                }
            });
    } catch (const std::exception &e) {
        std::cerr << "Exception in async_send_to: " << e.what() << std::endl;
    }
}

void Network::SendUdp(const std::array<uint8_t, MAX_UDP_PACKET_SIZE> &data,
    std::size_t size, const boost::asio::ip::udp::endpoint &endpoint) {
    udp_.Send(data, size, endpoint);
}
}  // namespace server
