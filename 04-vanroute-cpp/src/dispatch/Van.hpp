#pragma once

#include <string>

namespace vanroute {

// A delivery van. Times are minutes after midnight.
struct Van {
  std::string id;
  std::string depot;        // road-network node where the shift starts
  int capacity = 0;         // capacity units
  int shiftStart = 8 * 60;  // 08:00
  int shiftEnd = 18 * 60;   // 18:00: every delivery must be finished by then

  // Throws std::invalid_argument if capacity <= 0 or the shift doesn't end after it starts.
  Van(std::string id, std::string depot, int capacity, int shiftStart, int shiftEnd);
};

}  // namespace vanroute
