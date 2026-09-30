#include "report/Departures.hpp"

#include <unordered_map>

namespace shiftplan {

std::vector<TruckStatus> truckStatuses(const std::vector<Truck>& trucks, const ShiftPlan& plan, const Shift& shift) {
  std::unordered_map<std::string, int> finishOf;
  for (const Slot& slot : plan.slots) finishOf[slot.job] = slot.finish;

  std::vector<TruckStatus> board;
  for (const Truck& truck : trucks) {
    TruckStatus status;
    status.truck = truck.id;
    status.loadJob = truck.loadJob;
    status.cutoff = truck.cutoff - shift.start;  // both in minutes, so the difference is minutes into the shift

    const auto it = finishOf.find(truck.loadJob);
    if (it == finishOf.end()) {
      status.state = TruckState::CarriedOver;
    } else {
      status.sealedAt = it->second + kSealMinutes;
      status.state = *status.sealedAt <= status.cutoff ? TruckState::OnTime : TruckState::Late;
    }
    board.push_back(status);
  }
  return board;
}

}  // namespace shiftplan
