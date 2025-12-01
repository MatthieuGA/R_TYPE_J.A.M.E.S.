#pragma once
#include "Engine/Events/Event.h"
#include "Engine/gameWorld.hpp"

// Lists of events
struct CollisionEvent : public Event {
    int entityA;
    int entityB;
    Rtype::Client::GameWorld &gameWorld;
    CollisionEvent(int a, int b, Rtype::Client::GameWorld &gw)
        : entityA(a), entityB(b), gameWorld(gw) {}
};

