#pragma once
#include "Engine/Audio/AudioManager.hpp"
#include "Engine/gameWorld.hpp"

namespace Rtype::Client {
namespace RC = Rtype::Client;

void InitRegistry(
    Rtype::Client::GameWorld &game_world, Audio::AudioManager &audio_manager);

}  // namespace Rtype::Client
