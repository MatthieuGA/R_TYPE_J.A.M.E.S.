#include <iostream>
#include <utility>

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
                     config.GetUdpPort())) {}

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
                    // Process and deserialize received UDP input packets (push
                    // to SPSC queue, etc.) Specific function will use
                    // bytes_recvd to know the actual content of the packet
                } else {
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
}  // namespace server
