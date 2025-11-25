#include <iostream>

#include <boost/asio.hpp>
#include <server/Config.hpp>
#include <server/Network.hpp>

int main(int argc, char *argv[]) {
    try {
        boost::asio::io_context io;
        server::Config &cfg = server::Config::fromCommandLine(argc, argv);
        server::Network network = server::Network(cfg, io);
        // TODO(someone): Add Game class to manage game state and logic, give
        // it a reference to Network so it can access the SPSC queue. Also
        // give it the io_context so it can set timers for game ticks and
        // use async_wait.

        std::cout << "Server started, running io_context..." << std::endl;
        io.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
