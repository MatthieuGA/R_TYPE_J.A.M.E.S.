/**
 * @file audioSystem.cpp
 * @brief Audio system that processes SoundRequest components.
 */

#include <vector>

#include "engine/audio/AudioManager.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {

/**
 * @brief Audio system implementation.
 *
 * This system:
 * - Iterates entities with SoundRequest components
 * - Calls audioManager.PlaySound() for each request
 * - Removes the SoundRequest component after processing
 *
 * It does NOT interact with SFML directly.
 * It does NOT know about physics, rendering, or networking.
 *
 * @param reg The ECS registry.
 * @param audio_manager The audio manager instance.
 * @param sound_requests Sparse array of SoundRequest components.
 */
void AudioSystem(Engine::registry &reg, Audio::AudioManager &audio_manager,
    Engine::sparse_array<Component::SoundRequest> &sound_requests) {
    std::vector<size_t> entities_to_clear;

    // Process all sound requests
    for (auto &&[entity_index, request] :
        Engine::indexed_zipper(sound_requests)) {
        // Play the sound through the manager
        audio_manager.PlaySound(request.sound_id, request.volume);

        // Mark entity for component removal
        entities_to_clear.push_back(entity_index);
    }

    // Remove all processed SoundRequest components
    for (const auto &entity_id : entities_to_clear) {
        reg.RemoveComponent<Component::SoundRequest>(
            reg.EntityFromIndex(entity_id));
    }

    // Update the audio backend
    audio_manager.Update();
}

}  // namespace Rtype::Client
