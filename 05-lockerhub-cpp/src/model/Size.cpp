#include "model/Size.hpp"

#include <stdexcept>

namespace lockerhub {

Size parseSize(const std::string& code) {
  if (code == "S") return Size::Small;
  if (code == "M") return Size::Medium;
  if (code == "L") return Size::Large;
  throw std::invalid_argument("unknown size code '" + code + "' (expected S, M or L)");
}

char sizeCode(Size size) {
  switch (size) {
    case Size::Small:
      return 'S';
    case Size::Medium:
      return 'M';
    case Size::Large:
      return 'L';
  }
  return '?';
}

bool fits(Size parcel, Size compartment) {
  return static_cast<int>(parcel) <= static_cast<int>(compartment);
}

}  // namespace lockerhub
