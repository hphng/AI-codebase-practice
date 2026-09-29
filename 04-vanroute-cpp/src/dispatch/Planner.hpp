#pragma once

#include <string>
#include <vector>

#include "dispatch/TravelOracle.hpp"
#include "dispatch/Van.hpp"
#include "model/Parcel.hpp"

namespace vanroute {

struct Stop {
  std::string parcelId;
  std::string destination;
  int arrival = 0;  // minutes after midnight
  bool late = false;
};

struct VanPlan {
  std::string vanId;
  int capacity = 0;
  int load = 0;
  std::vector<Stop> stops;
};

struct Unassigned {
  std::string parcelId;
  std::string reason;
};

struct Plan {
  std::vector<VanPlan> vans;  // same order as the input vans
  std::vector<Unassigned> unassigned;
};

// Greedy dispatch:
//  1. Cancelled parcels are ignored.
//  2. Parcels are taken in dispatch order (earliest deadline first, ties by id).
//  3. Each parcel goes to the FIRST van (input order) that
//       - has enough capacity left, and
//       - can reach the parcel's destination from where it is now, and
//       - can finish the delivery (arrival + service time) by the end of its shift.
//     The van then drives there: arrival = its clock + travel minutes, and its clock becomes
//     arrival + service time. Vans start at their depot at shiftStart.
//  4. A stop is "late" if arrival > deadline (it's still delivered).
//  5. Parcels no van can take are listed in `unassigned`.
class Planner {
 public:
  explicit Planner(const TravelOracle& oracle, int serviceMinutes = 5)
      : oracle_(oracle), serviceMinutes_(serviceMinutes) {}

  Plan plan(const std::vector<Van>& vans, const std::vector<Parcel>& parcels) const;

 private:
  int travelMinutes(const std::string& from, const std::string& to) const;

  const TravelOracle& oracle_;
  int serviceMinutes_;
};

}  // namespace vanroute
