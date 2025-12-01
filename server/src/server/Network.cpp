#include <iostream>

#include <server/Network.hpp>

namespace server {
Network::Network(Config &config, boost::asio::io_context &io)
    : tcpAcceptor(
          io, boost::asio::ip::tcp::endpoint(
                  boost::asio::ip::make_address(config.getTcpAddress()),
                  config.getTcpPort())),
      udpSocket(io, boost::asio::ip::udp::endpoint(
                        boost::asio::ip::make_address(config.getUdpAddress()),
                        config.getUdpPort())),
      udpRemoteEndpoint(),
      udpBuffer() {
    std::cout << "Waiting for TCP connections on port "
              << tcpAcceptor.local_endpoint().port() << "..." << std::endl;
    std::cout << "Waiting for UDP packets on port "
              << udpSocket.local_endpoint().port() << "..." << std::endl;
    do_tcp_accept();
    do_udp_receive();
}

void Network::do_tcp_accept() {
    try {
        tcpAcceptor.async_accept([this](boost::system::error_code ec,
                                     boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                std::cout << "New TCP connection from "
                          << socket.remote_endpoint().address().to_string()
                          << ":" << socket.remote_endpoint().port()
                          << std::endl;
                // Later: store socket in a connection manager, start reading,
                // etc. auto connection =
                // std::make_shared<TcpConnection>(std::move(socket));
                // connection->start();
            } else {
                std::cerr << "Error accepting TCP connection: " << ec.message()
                          << std::endl;
            }
            do_tcp_accept();  // Accept next connection (async chain)
        });
    } catch (const std::exception &e) {
        std::cerr << "Exception in async_accept: " << e.what() << std::endl;
    }
}

void Network::do_udp_receive() {
    try {
        udpSocket.async_receive_from(udpBuffer, udpRemoteEndpoint,
            [this](boost::system::error_code ec, std::size_t bytes_recvd) {
                if (!ec && bytes_recvd > 0) {
                    std::cout << "Received UDP packet from "
                              << udpRemoteEndpoint.address().to_string() << ":"
                              << udpRemoteEndpoint.port()
                              << ", size: " << bytes_recvd << " bytes"
                              << std::endl;
                    // Process received UDP packet (push to SPSC queue, etc.)
                    // Specific function will use bytes_recvd to know the
                    // actual content of the packet
                } else {
                    std::cerr << "Error receiving UDP packet: " << ec.message()
                              << std::endl;
                }
                do_udp_receive();  // Receive next UDP packet (async chain)
            });
    } catch (const std::exception &e) {
        std::cerr << "Exception in async_receive_from: " << e.what()
                  << std::endl;
    }
}
}  // namespace server
