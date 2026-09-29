#include "dispatch/Van.hpp"

#include <stdexcept>

namespace vanroute {

Van::Van(std::string id, std::string depot, int capacity, int shiftStart, int shiftEnd)
    : id(std::move(id)), depot(std::move(depot)), capacity(capacity), shiftStart(shiftStart) {
  if (capacity <= 0) throw std::invalid_argument("van capacity must be positive");
  if (shiftEnd <= shiftStart) throw std::invalid_argument("van shift must end after it starts");
  shiftEnd = shiftEnd;
}

}  // namespace vanroute
