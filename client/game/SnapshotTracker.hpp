#pragma once
#include <cstdint>

class SnapshotTracker {
 public:
    SnapshotTracker() = default;

    static SnapshotTracker &GetInstance() {
        static SnapshotTracker instance;
        return instance;
    }

    void UpdateLastProcessedTick(uint32_t tick) {
        last_processed_tick_ = tick;
    }

    uint32_t GetLastProcessedTick() const {
        return last_processed_tick_;
    }

 private:
    uint32_t last_processed_tick_{0};
};
