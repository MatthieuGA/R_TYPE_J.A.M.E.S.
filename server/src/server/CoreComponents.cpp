#include "server/CoreComponents.hpp"

namespace server::Component {

float Transform::GetWorldRotation() const {
    return rotationDegrees;
}

}  // namespace server::Component
