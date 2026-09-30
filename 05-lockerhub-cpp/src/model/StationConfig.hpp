#pragma once

#include <string>

namespace lockerhub {

// data/station.csv: one row describing the station.
struct StationConfig {
  std::string id;        // "HUB-SEA-14"
  std::string name;      // "Capitol Hill Hub"
  int holdHours = 72;    // how long a customer has to pick a parcel up before it goes back
};

}  // namespace lockerhub
