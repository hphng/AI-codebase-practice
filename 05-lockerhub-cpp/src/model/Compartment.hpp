#pragma once

#include <string>

#include "model/Size.hpp"

namespace lockerhub {

// One door in the locker bank, e.g. {"A03", Size::Small}.
struct Compartment {
  std::string id;
  Size size = Size::Small;
};

}  // namespace lockerhub
