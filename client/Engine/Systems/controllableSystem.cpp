#include "Engine/initRegisterySystems.hpp"
#include "include/indexed_zipper.hpp"
#include <iostream>

using namespace Engine;

namespace Rtype::Client {
    using namespace Component;

void controllableSystem(registry &reg,
sparse_array<Controllable> const &controls) {
    for (auto &&[i, control] : make_indexed_zipper(controls)) {
        if (control.isControllable) {
            //  std::cerr << "Entity " << i << " is controllable.\n";
        }
    }
}
}  // namespace Rtype::Client
