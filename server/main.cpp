#include <iostream>

#include <boost/asio.hpp>
#include <server/Config.hpp>
#include <server/Server.hpp>

int main(int argc, char *argv[]) {
    try {
        boost::asio::io_context io;
        server::Config &cfg = server::Config::fromCommandLine(argc, argv);
        server::Server game_server(cfg, io);

        game_server.initialize();
        game_server.start();

        std::cout << "Server started, running io_context..." << std::endl;
        io.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
