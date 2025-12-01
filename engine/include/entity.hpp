#pragma once

#include <cstddef>

namespace Engine {
class registry;  // forward declaration

class entity {
 public:
    virtual ~entity() = default;

    entity &operator=(size_t new_id);
    size_t getId() const { return id_; }
    
    // Comparison operators for sorting and equality
    bool operator<(const entity& other) const { return id_ < other.id_; }
    bool operator==(const entity& other) const { return id_ == other.id_; }
    bool operator!=(const entity& other) const { return id_ != other.id_; }

 private:
    explicit entity(size_t id) : id_(id) {}
    friend class registry;  // only registry can construct entities

    size_t id_;
};
}  // namespace Engine
