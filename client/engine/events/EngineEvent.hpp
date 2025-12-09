#pragma once
#include "engine/gameWorld.hpp"

#include "engine/events/Event.h"

// Lists of events
struct CollisionEvent : public Event {
    size_t entity_a_;
    size_t entity_b_;
    Rtype::Client::GameWorld &game_world_;

    CollisionEvent(size_t a, size_t b, Rtype::Client::GameWorld &gw)
        : entity_a_(a), entity_b_(b), game_world_(gw) {}
};
