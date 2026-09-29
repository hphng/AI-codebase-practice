#include "fraud/Households.hpp"

namespace returns {

std::unordered_map<std::string, std::string> buildHouseholds(const std::vector<Customer>& customers) {
  // TODO(Part A): replace this placeholder. Right now every customer is treated as
  // their own household, which is what lets multi-account abuse through.
  std::unordered_map<std::string, std::string> household;
  for (const Customer& c : customers) household[c.id] = c.id;
  return household;
}

}  // namespace returns
