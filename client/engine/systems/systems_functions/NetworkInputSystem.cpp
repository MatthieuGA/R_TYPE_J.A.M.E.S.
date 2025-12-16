#include <iostream>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {

// Track last sent input to reduce logging spam
static uint8_t last_sent_input_ = 0;

/**
 * @brief Send player input packets to the server via UDP.
 *
 * This system converts the Inputs component to an RFC-compliant bitfield
 * format and sends it to the server using the PLAYER_INPUT packet (0x10).
 * Packets are sent every tick for entities with both PlayerTag and Inputs
 * components.
 *
 * According to RFC Section 6.1, the bitfield format is:
 * - Bit 0: Up
 * - Bit 1: Down
 * - Bit 2: Left
 * - Bit 3: Right
 * - Bit 4: Shoot
 *
 * @param reg Engine registry (unused)
 * @param game_world GameWorld containing the ServerConnection
 * @param inputs Sparse array of Inputs components
 * @param player_tags Sparse array of PlayerTag components (filters player
 * entities)
 */
void NetworkInputSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::PlayerTag> const &player_tags) {
    // Only send if connected to server
    if (!game_world.server_connection_ ||
        !game_world.server_connection_->is_connected()) {
        return;
    }

    // Poll io_context to process async network operations (non-blocking)
    game_world.io_context_.poll();

    // Send input for each player-controlled entity
    for (auto &&[i, input, player_tag] :
        make_indexed_zipper(inputs, player_tags)) {
        uint8_t input_bitfield = InputToBitfield(input);

        // Send input packet via UDP
        game_world.server_connection_->SendInput(input_bitfield);

        // Log packet transmission only when input changes (reduce spam)
        if (input_bitfield != last_sent_input_) {
            // std::cout << "[Network] Sent PLAYER_INPUT: bitfield=0x" <<
            // std::hex
            //           << static_cast<int>(input_bitfield) << std::dec
            //           << " (Up=" << ((input_bitfield & (1 << 0)) ? "1" :
            //           "0")
            //           << " Down=" << ((input_bitfield & (1 << 1)) ? "1" :
            //           "0")
            //           << " Left=" << ((input_bitfield & (1 << 2)) ? "1" :
            //           "0")
            //           << " Right=" << ((input_bitfield & (1 << 3)) ? "1" :
            //           "0")
            //           << " Shoot=" << ((input_bitfield & (1 << 4)) ? "1" :
            //           "0")
            //           << ")" << std::endl;
            last_sent_input_ = input_bitfield;
        }
    }
}
}  // namespace Rtype::Client
