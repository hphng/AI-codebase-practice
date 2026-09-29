#pragma once

#include <string>

namespace vanroute {

struct Parcel {
  std::string id;
  std::string destination;  // road-network node
  int size = 1;             // capacity units
  int deadline = 0;         // minutes after midnight
  bool cancelled = false;
};

}  // namespace vanroute
