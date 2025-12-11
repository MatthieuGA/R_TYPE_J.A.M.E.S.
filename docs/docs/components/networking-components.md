# Networking Components

**Source file:** `client/include/components/NetworkingComponents.hpp`

**Purpose:** Carry the network identity of an entity and client-side interpolation data for smoother rendering.

## NetworkId
- Network identifier `id` used to match local entities with server-replicated ones.

## InterpolatedPosition
- Target position `goalPosition` received from the server.
- Interpolation speed `speed` used by client smoothing systems to converge the local transform toward the authoritative state.
