#include <iostream>

#include <boost/asio.hpp>
#include <server/Config.hpp>
#include <server/Server.hpp>

int main(int argc, char *argv[]) {
    try {
        boost::asio::io_context io;
        server::Config &cfg = server::Config::FromCommandLine(argc, argv);
        server::Server game_server(cfg, io);

        // Setup signal handling for graceful shutdown (Ctrl+C, SIGTERM)
        boost::asio::signal_set signals(io, SIGINT, SIGTERM);
        signals.async_wait(
            [&game_server, &io](
                const boost::system::error_code &ec, int signal_number) {
                if (!ec) {
                    std::cout << "\nReceived signal " << signal_number
                              << ", shutting down gracefully..." << std::endl;
                    game_server.Close();
                    io.stop();
                }
            });

        game_server.Initialize();

        // Prompt for level selection before starting
        game_server.PromptLevelSelection();

        std::cout << "Server setup, running io_context..." << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        io.run();  // Blocks until io_context is stopped
        std::cout << "io_context stopped, server shutdown complete."
                  << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
