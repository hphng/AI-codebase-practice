#pragma once

#include <map>
#include <string>
#include <vector>

namespace shiftplan {

// One piece of work in the shift: unload a trailer, sort a lane, load a truck, ...
struct Job {
  std::string id;                  // "U1", "S3", "L-T2"
  int minutes = 0;                 // how long it takes one crew
  int priority = 0;                // higher goes first
  std::string crew;                // crew type that does it: "unloader", "sorter", "loader"
  std::vector<std::string> after;  // jobs that must be finished before this one can start
};

// Crew type -> number of crews of that type on shift.
using CrewPool = std::map<std::string, int>;

}  // namespace shiftplan
