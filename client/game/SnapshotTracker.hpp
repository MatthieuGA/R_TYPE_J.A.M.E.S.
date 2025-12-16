#pragma once
#include <atomic>
#include <cstdint>

/**
 * @brief Tracks the last processed snapshot tick on the client.
 *
 * @note Thread-safe for concurrent reads/writes via atomic tick storage.
 */
class SnapshotTracker {
 public:
    SnapshotTracker() = default;

    /**
     * @brief Provides the singleton instance.
     *
     * @return Reference to the unique instance.
     */
    static SnapshotTracker &GetInstance() {
        static SnapshotTracker instance;
        return instance;
    }

    /**
     * @brief Updates the last processed snapshot tick.
     *
     * @param tick Latest processed tick value.
     */
    void UpdateLastProcessedTick(uint32_t tick) {
        last_processed_tick_.store(tick, std::memory_order_relaxed);
    }

    /**
     * @brief Retrieves the last processed snapshot tick.
     *
     * @return Stored tick value.
     */
    uint32_t GetLastProcessedTick() const {
        return last_processed_tick_.load(std::memory_order_relaxed);
    }

 private:
    std::atomic<uint32_t> last_processed_tick_{0};
};
