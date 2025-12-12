#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "server/Vector2f.hpp"

namespace server::Component {

struct NetworkId {
    uint8_t id = 0;
};

}  // namespace server::Component
