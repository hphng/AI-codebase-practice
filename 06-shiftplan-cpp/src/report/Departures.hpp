#pragma once

#include <optional>
#include <string>
#include <vector>

#include "model/Shift.hpp"
#include "model/Truck.hpp"
#include "plan/Scheduler.hpp"

namespace shiftplan {

// Outbound rule OUT-3: after its load job finishes, a trailer takes kSealMinutes to close and seal.
// A truck is on time if it is sealed at or before its cutoff. A truck whose load job doesn't run this
// shift is carried over.
constexpr int kSealMinutes = 15;

enum class TruckState { OnTime, Late, CarriedOver };

struct TruckStatus {
  std::string truck;
  std::string loadJob;
  int cutoff = 0;                // minutes since the shift started
  std::optional<int> sealedAt;   // minutes since the shift started; empty if carried over
  TruckState state = TruckState::CarriedOver;
};

// The departure board, one entry per truck in input order. Truck cutoffs are clock times and plan
// times are minutes since the shift started; see Shift in model/Shift.hpp for converting between them.
std::vector<TruckStatus> truckStatuses(const std::vector<Truck>& trucks, const ShiftPlan& plan, const Shift& shift);

}  // namespace shiftplan
