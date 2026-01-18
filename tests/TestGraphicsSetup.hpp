/**
 * @file TestGraphicsSetup.hpp
 * @brief Test setup helper for graphics backend registration.
 *
 * Provides utilities to register mock backends for testing.
 *
 * SCOPE: PR 1.9 - Test infrastructure.
 */

#ifndef TESTS_TESTGRAPHICSSETUP_HPP_
#define TESTS_TESTGRAPHICSSETUP_HPP_

#include <memory>

#include "graphics/GraphicsBackendFactory.hpp"
#include "mock/MockRenderContext.hpp"

namespace TestHelper {

/**
 * @brief Register the mock render context as "test" backend.
 *
 * Should be called before creating GameWorld in tests.
 * Safe to call multiple times (later registrations override earlier ones).
 */
inline void RegisterTestBackend() {
    Rtype::Client::Graphics::GraphicsBackendFactory::Register(
        "test", [](sf::RenderWindow &window) {
            return std::make_unique<
                Rtype::Client::Graphics::MockRenderContext>();
        });
}

}  // namespace TestHelper

#endif  // TESTS_TESTGRAPHICSSETUP_HPP_
