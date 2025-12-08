#include <iostream>
#include <utility>

#include <server/Network.hpp>

namespace server {
Network::Network(Config &config, boost::asio::io_context &io)
    : _tcp(config, io), _udp(config, io) {
    std::cout << "Waiting for TCP connections on port " << _tcp.port() << "..."
              << std::endl;
    std::cout << "Waiting for UDP packets on port " << _udp.port() << "..."
              << std::endl;
    _tcp.accept();
    _udp.receive();
}

Network::UDP::UDP(Config &config, boost::asio::io_context &io)
    : socket(io, boost::asio::ip::udp::endpoint(
                     boost::asio::ip::make_address(config.getUdpAddress()),
                     config.getUdpPort())) {}

Network::TCP::TCP(Config &config, boost::asio::io_context &io)
    : acceptor(io, boost::asio::ip::tcp::endpoint(
                       boost::asio::ip::make_address(config.getTcpAddress()),
                       config.getTcpPort())) {}

boost::asio::ip::port_type Network::UDP::port() const {
    return socket.local_endpoint().port();
}

boost::asio::ip::port_type Network::TCP::port() const {
    return acceptor.local_endpoint().port();
}

void Network::TCP::accept() {
    try {
        acceptor.async_accept([this](boost::system::error_code ec,
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
            accept();  // Accept next connection (async chain)
        });
    } catch (const std::exception &e) {
        std::cerr << "Exception in async_accept: " << e.what() << std::endl;
    }
}

void Network::UDP::receive() {
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
                receive();  // Receive next UDP packet (async chain)
            });
    } catch (const std::exception &e) {
        std::cerr << "Exception in async_receive_from: " << e.what()
                  << std::endl;
    }
}
}  // namespace server
