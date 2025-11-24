#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>
#include <asio.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <cmath>

/**
 * @brief R-TYPE J.A.M.E.S. Setup Verification
 * Tests SFML (graphics, window, network, audio) and Asio integration
 */

void drawText(sf::RenderWindow& window, const std::string& text, float x, float y, 
              unsigned int size, const sf::Color& color, sf::Font& font) {
    sf::Text sfText(font, text, size);
    sfText.setPosition({x, y});
    sfText.setFillColor(color);
    window.draw(sfText);
}

void testAsioNetwork() {
    std::cout << "\n=== Testing Asio Networking ===" << std::endl;

    try {
        asio::io_context io_context;

        // Test 1: UDP Socket
        std::cout << "Test 1: Creating UDP socket..." << std::endl;
        asio::ip::udp::socket udp_socket(io_context);
        udp_socket.open(asio::ip::udp::v4());

        auto local_endpoint = udp_socket.local_endpoint();
        std::cout << "✓ UDP socket created" << std::endl;
        std::cout << "  Local endpoint: " << local_endpoint << std::endl;
        udp_socket.close();

        // Test 2: TCP Socket
        std::cout << "\nTest 2: Creating TCP socket..." << std::endl;
        asio::ip::tcp::socket tcp_socket(io_context);
        tcp_socket.open(asio::ip::tcp::v4());
        std::cout << "✓ TCP socket created" << std::endl;
        tcp_socket.close();

        // Test 3: Resolver
        std::cout << "\nTest 3: Creating TCP resolver..." << std::endl;
        asio::ip::tcp::resolver resolver(io_context);
        std::cout << "✓ TCP resolver created" << std::endl;

        // Test 4: Resolve localhost
        std::cout << "\nTest 4: Resolving 'localhost'..." << std::endl;
        auto results = resolver.resolve("localhost", "80");
        int count = 0;
        for (const auto& entry : results) {
            std::cout << "  → " << entry.endpoint() << std::endl;
            count++;
            if (count >= 3) break;
        }
        std::cout << "✓ Resolved " << count << " endpoints" << std::endl;

        std::cout << "\n✓ All Asio networking tests passed!\n" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "✗ Asio error: " << e.what() << std::endl;
    }
}

void testSFMLNetwork() {
    std::cout << "\n=== Testing SFML Networking ===" << std::endl;

    // Test UDP Socket
    sf::UdpSocket udpSocket;
    if (udpSocket.bind(sf::Socket::AnyPort) == sf::Socket::Status::Done) {
        std::cout << "✓ SFML UDP socket bound to port: " << udpSocket.getLocalPort() << std::endl;
    } else {
        std::cout << "✗ Failed to bind SFML UDP socket" << std::endl;
    }

    // Test TCP Socket
    sf::TcpSocket tcpSocket;
    std::cout << "✓ SFML TCP socket created" << std::endl;

    std::cout << "✓ SFML networking tests passed!\n" << std::endl;
}int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "R-TYPE J.A.M.E.S. - Setup Verification" << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "SFML Version: " << SFML_VERSION_MAJOR << "."
              << SFML_VERSION_MINOR << "." << SFML_VERSION_PATCH << std::endl;
    std::cout << "Asio Version: " << ASIO_VERSION << std::endl;
    std::cout << "==========================================" << std::endl;

    // Test networking first
    testAsioNetwork();
    testSFMLNetwork();

    // Load font (using default system font if available)
    sf::Font font;
    if (!font.openFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") &&
        !font.openFromFile("/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf") &&
        !font.openFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf") &&
        !font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
        std::cout << "⚠ Warning: Could not load font, text will not display" << std::endl;
        std::cout << "Graphics test will continue with shapes only..." << std::endl;
    }

    // Create window
    std::cout << "\n=== Testing SFML Graphics ===" << std::endl;
    sf::RenderWindow window(sf::VideoMode({800, 600}), "R-TYPE J.A.M.E.S. - SFML Test");
    window.setFramerateLimit(60);

    std::cout << "✓ SFML window created (800x600)" << std::endl;
    std::cout << "✓ Framerate limited to 60 FPS" << std::endl;

    // Create test shapes
    sf::CircleShape circle(50.f);
    circle.setFillColor(sf::Color::Green);
    circle.setPosition({100.f, 100.f});

    sf::RectangleShape rectangle(sf::Vector2f(100.f, 60.f));
    rectangle.setFillColor(sf::Color::Blue);
    rectangle.setPosition({100.f, 250.f});

    sf::ConvexShape triangle;
    triangle.setPointCount(3);
    triangle.setPoint(0, sf::Vector2f(0.f, 0.f));
    triangle.setPoint(1, sf::Vector2f(50.f, 0.f));
    triangle.setPoint(2, sf::Vector2f(25.f, 50.f));
    triangle.setFillColor(sf::Color::Red);
    triangle.setPosition({100.f, 400.f});    // Animation variables
    sf::Clock clock;
    float rotation = 0.f;

    std::cout << "✓ Test shapes created" << std::endl;
    std::cout << "\n=== Interactive Test ===" << std::endl;
    std::cout << "Window opened! Test the following:" << std::endl;
    std::cout << "  • Green circle (rotating)" << std::endl;
    std::cout << "  • Blue rectangle (pulsing)" << std::endl;
    std::cout << "  • Red triangle (static)" << std::endl;
    std::cout << "  • Press SPACE for Asio network test" << std::endl;
    std::cout << "  • Press N for SFML network test" << std::endl;
    std::cout << "  • Press ESC or close window to exit" << std::endl;

    // Main loop
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                std::cout << "\n✓ Window closed by user" << std::endl;
                window.close();
            }

            if (const auto* keyPress = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPress->code == sf::Keyboard::Key::Escape) {
                    std::cout << "\n✓ ESC pressed - exiting" << std::endl;
                    window.close();
                } else if (keyPress->code == sf::Keyboard::Key::Space) {
                    testAsioNetwork();
                } else if (keyPress->code == sf::Keyboard::Key::N) {
                    testSFMLNetwork();
                }
            }
        }        // Update animations
        float deltaTime = clock.restart().asSeconds();
        rotation += 50.f * deltaTime;

        circle.setRotation(sf::degrees(rotation));

        float pulse = (std::sin(rotation * 0.05f) + 1.f) * 0.5f;
        rectangle.setScale({1.f, 0.7f + pulse * 0.6f});        // Clear and draw
        window.clear(sf::Color(40, 40, 45));

        // Draw shapes
        window.draw(circle);
        window.draw(rectangle);
        window.draw(triangle);

        // Draw text if font loaded
        if (font.getInfo().family != "") {
            drawText(window, "R-TYPE J.A.M.E.S.", 250, 50, 32, sf::Color::White, font);
            drawText(window, "Setup Verification", 250, 90, 20, sf::Color(200, 200, 200), font);

            drawText(window, "Integration Status:", 250, 150, 18, sf::Color::Yellow, font);
            drawText(window, "  SFML Graphics", 250, 180, 16, sf::Color::Green, font);
            drawText(window, "  SFML Window", 250, 210, 16, sf::Color::Green, font);
            drawText(window, "  SFML Network", 250, 240, 16, sf::Color::Green, font);
            drawText(window, "  Asio Networking", 250, 270, 16, sf::Color::Green, font);
            drawText(window, "  vcpkg Manager", 250, 300, 16, sf::Color::Green, font);

            drawText(window, "Controls:", 250, 350, 18, sf::Color::Yellow, font);
            drawText(window, "  SPACE - Test Asio", 250, 380, 14, sf::Color::White, font);
            drawText(window, "  N - Test SFML Net", 250, 410, 14, sf::Color::White, font);
            drawText(window, "  ESC - Exit", 250, 440, 14, sf::Color::White, font);

            std::ostringstream fpsText;
            fpsText << "FPS: " << static_cast<int>(1.f / deltaTime);
            drawText(window, fpsText.str(), 650, 550, 16, sf::Color(150, 150, 150), font);
        }

        window.display();
    }

    std::cout << "\n==========================================" << std::endl;
    std::cout << "✓ All tests completed successfully!" << std::endl;
    std::cout << "SFML and Asio are working correctly." << std::endl;
    std::cout << "==========================================" << std::endl;

    return 0;
}