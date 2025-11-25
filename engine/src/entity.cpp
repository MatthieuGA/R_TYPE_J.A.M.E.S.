#include "../include/entity.hpp"

namespace Engine {
entity &entity::operator=(size_t new_id) {
    this->id_ = new_id;
    return *this;
}
}  // namespace Engine
