#pragma once

#include <cstddef>

namespace ecs {

class entity {
public:
    using id_type = std::size_t;

    explicit entity(id_type id) : _id(id) {}

    id_type id() const noexcept {
        return _id;
    }

    operator id_type() const noexcept {
        return _id;
    }

    bool operator==(const entity& other) const noexcept {
        return _id == other._id;
    }

    bool operator!=(const entity& other) const noexcept {
        return _id != other._id;
    }

    bool operator<(const entity& other) const noexcept {
        return _id < other._id;
    }

private:
    id_type _id;
};

} // namespace ecs
