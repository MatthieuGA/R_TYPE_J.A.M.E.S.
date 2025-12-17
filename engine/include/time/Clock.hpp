/**
 * @file Clock.hpp
 * @brief Engine-agnostic clock/time abstraction.
 */

#ifndef ENGINE_INCLUDE_TIME_CLOCK_HPP_
#define ENGINE_INCLUDE_TIME_CLOCK_HPP_

#include <chrono>

namespace Engine {
namespace Time {

/**
 * @brief Time duration in seconds (float).
 */
class Duration {
 public:
    Duration() : seconds_(0.0f) {}

    explicit Duration(float seconds) : seconds_(seconds) {}

    float AsSeconds() const {
        return seconds_;
    }

    int AsMilliseconds() const {
        return static_cast<int>(seconds_ * 1000.0f);
    }

    int64_t AsMicroseconds() const {
        return static_cast<int64_t>(seconds_ * 1000000.0f);
    }

 private:
    float seconds_;
};

/**
 * @brief Simple clock for measuring elapsed time.
 */
class Clock {
 public:
    Clock() : start_(std::chrono::steady_clock::now()) {}

    /**
     * @brief Get elapsed time since last restart.
     * @return Duration since last restart or construction.
     */
    Duration GetElapsedTime() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            now - start_);
        return Duration(elapsed.count() / 1000000.0f);
    }

    /**
     * @brief Restart the clock and return elapsed time since last restart.
     * @return Duration since last restart.
     */
    Duration Restart() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            now - start_);
        start_ = now;
        return Duration(elapsed.count() / 1000000.0f);
    }

 private:
    std::chrono::steady_clock::time_point start_;
};

}  // namespace Time
}  // namespace Engine

#endif  // ENGINE_INCLUDE_TIME_CLOCK_HPP_
