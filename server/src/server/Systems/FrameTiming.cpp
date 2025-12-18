#include <algorithm>

#include "server/systems/Systems.hpp"

namespace server {

// Define frame delta globals
float g_frame_delta_ms = 16.0f;
float g_frame_delta_seconds = 16.0f / 1000.0f;

void UpdateFrameDeltaFromSeconds(float seconds) {
    // Enforce maximum 60 FPS (minimum delta)
    if (seconds < kMinFrameDeltaSeconds)
        seconds = kMinFrameDeltaSeconds;
    g_frame_delta_seconds = seconds;
    g_frame_delta_ms = seconds * 1000.0f;
}

}  // namespace server
