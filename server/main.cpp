#include <iostream>

#include <boost/asio.hpp>
#include <server/Config.hpp>

int main(int argc, char *argv[]) {
    try {
        server::Config &config = server::Config::fromCommandLine(argc, argv);

        // SPSC ring buffer queue initialization
        // ASIO: TCP acceptor setup for client connections

        // ASIO: UDP socket setup with handle packet function in a separate
        // thread. It pushes events to the main thread via the SPSC queue and
        // drops new events if it's full.

        // R-Type: Event loop processing events from the queue and handling
        // game logic, also handles sending UDP snapshots to clients. Singleton
        // class
        std::cout << "Server is running..." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
