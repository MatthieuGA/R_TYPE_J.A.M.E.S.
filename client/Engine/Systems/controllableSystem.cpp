#include <iostream>

#include "Engine/initRegistrySystems.hpp"
#include "indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

void controllableSystem(
    Eng::registry &reg, Eng::sparse_array<Com::Controllable> const &controls) {
    for (auto &&[i, control] : make_indexed_zipper(controls)) {
        if (control.isControllable) {
            //  std::cerr << "Entity " << i << " is controllable.\n";
        }
    }
}
}  // namespace Rtype::Client
