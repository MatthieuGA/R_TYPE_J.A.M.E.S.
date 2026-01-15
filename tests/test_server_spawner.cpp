#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "game/ServerSpawner.hpp"

namespace RC = Rtype::Client;

// =============================================================================
// Port Availability Tests
// =============================================================================

TEST(ServerSpawner, PortAvailabilityConstants) {
    // Verify the port range constants are reasonable
    EXPECT_GE(RC::ServerSpawner::kStartPort, 1024);  // Above privileged
    EXPECT_LE(RC::ServerSpawner::kMaxPort, 65535);   // Valid port range
    EXPECT_LT(RC::ServerSpawner::kStartPort, RC::ServerSpawner::kMaxPort);
}

TEST(ServerSpawner, InitialStateNotRunning) {
    // Server should not be running initially
    EXPECT_FALSE(RC::ServerSpawner::IsServerRunning());
    EXPECT_EQ(RC::ServerSpawner::GetServerPort(), 0);
}

// =============================================================================
// FindServerExecutable Tests
// =============================================================================

// Note: These tests verify behavior without actually spawning processes
// Full integration tests would require a test server binary

TEST(ServerSpawner, GetServerPortReturnsZeroWhenNotRunning) {
    // When no server is running, port should be 0
    // This doesn't start a server, just checks initial state
    if (!RC::ServerSpawner::IsServerRunning()) {
        EXPECT_EQ(RC::ServerSpawner::GetServerPort(), 0);
    }
}

// =============================================================================
// Thread Safety Tests
// =============================================================================

TEST(ServerSpawner, ConcurrentIsServerRunningCalls) {
    // Verify IsServerRunning can be called from multiple threads safely
    std::vector<std::thread> threads;
    std::atomic<int> call_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&call_count]() {
            for (int j = 0; j < 100; ++j) {
                (void)RC::ServerSpawner::IsServerRunning();
                call_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    EXPECT_EQ(call_count.load(), 1000);
}

TEST(ServerSpawner, ConcurrentGetServerPortCalls) {
    // Verify GetServerPort can be called from multiple threads safely
    std::vector<std::thread> threads;
    std::atomic<int> call_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&call_count]() {
            for (int j = 0; j < 100; ++j) {
                (void)RC::ServerSpawner::GetServerPort();
                call_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    EXPECT_EQ(call_count.load(), 1000);
}

// =============================================================================
// TerminateServer Safety Tests
// =============================================================================

TEST(ServerSpawner, TerminateServerWhenNotRunningIsNoOp) {
    // Calling TerminateServer when no server is running should be safe
    EXPECT_NO_THROW(RC::ServerSpawner::TerminateServer());
    EXPECT_FALSE(RC::ServerSpawner::IsServerRunning());
}

TEST(ServerSpawner, MultipleTerminateCallsAreSafe) {
    // Multiple TerminateServer calls should be safe (idempotent)
    for (int i = 0; i < 5; ++i) {
        EXPECT_NO_THROW(RC::ServerSpawner::TerminateServer());
    }
    EXPECT_FALSE(RC::ServerSpawner::IsServerRunning());
}

// =============================================================================
// Signal Handler Tests
// =============================================================================

TEST(ServerSpawner, SetupSignalHandlersDoesNotThrow) {
    // SetupSignalHandlers should not throw
    EXPECT_NO_THROW(RC::ServerSpawner::SetupSignalHandlers());
}

TEST(ServerSpawner, MultipleSignalHandlerSetupsAreSafe) {
    // Multiple calls to SetupSignalHandlers should be safe
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_THROW(RC::ServerSpawner::SetupSignalHandlers());
    }
}

// =============================================================================
// ServerGuard RAII Tests
// =============================================================================

TEST(ServerGuard, ConstructsWithSoloModeFalse) {
    // ServerGuard with solo_mode=false should not attempt termination
    EXPECT_NO_THROW({
        RC::ServerGuard guard(false);
        // Destructor called here - should be no-op
    });
}

TEST(ServerGuard, ConstructsWithSoloModeTrue) {
    // ServerGuard with solo_mode=true should call TerminateServer on destruct
    // Since no server is running, this should be a safe no-op
    EXPECT_NO_THROW({
        RC::ServerGuard guard(true);
        // Destructor called here - should call TerminateServer
    });
}

TEST(ServerGuard, NonCopyable) {
    // Verify ServerGuard is non-copyable at compile time
    // This is a static assertion test - if it compiles, it passes
    static_assert(!std::is_copy_constructible_v<RC::ServerGuard>,
        "ServerGuard should not be copy constructible");
    static_assert(!std::is_copy_assignable_v<RC::ServerGuard>,
        "ServerGuard should not be copy assignable");
}
