#include "server/systems/Systems.hpp"

namespace server {

// Provide a single-definition of globals to satisfy linkers that expect
// an object file symbol. The header contains inline defaults for
// translation units, but some build setups/tools may still require a
// single, non-inline definition for certain linkers; providing this
// file ensures compatibility.

// This file intentionally left blank: definitions moved to
// FrameTiming.cpp to provide a single, canonical definition.
// (Kept to satisfy build systems that may list this file.)

}  // namespace server
