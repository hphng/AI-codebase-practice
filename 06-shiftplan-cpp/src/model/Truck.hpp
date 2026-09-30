#pragma once

#include <string>

namespace shiftplan {

// An outbound truck. It can leave once its load job is done and the trailer is sealed.
struct Truck {
  std::string id;       // "T1"
  std::string loadJob;  // id of the job that loads it
  int cutoff = 0;       // clock time it must leave by, minutes after midnight (03:30 -> 210)
};

}  // namespace shiftplan
