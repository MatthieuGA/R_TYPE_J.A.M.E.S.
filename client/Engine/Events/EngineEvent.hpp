#pragma once
#include "Engine/Events/Event.h"
#include "Engine/gameWorld.hpp"

// Lists of events
struct CollisionEvent : public Event {
    size_t entityA;
    size_t entityB;
    Rtype::Client::GameWorld &gameWorld;
    CollisionEvent(size_t a, size_t b, Rtype::Client::GameWorld &gw)
        : entityA(a), entityB(b), gameWorld(gw) {}
};

