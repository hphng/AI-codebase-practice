#include "dispatch/Planner.hpp"

#include <map>
#include <utility>

#include "dispatch/DispatchQueue.hpp"

namespace vanroute {

int Planner::travelMinutes(const std::string& from, const std::string& to) const {
  // The same legs are looked up many times while planning a day, so remember them.
  static std::map<std::pair<std::string, std::string>, int> cache;

  const auto key = std::make_pair(from, to);
  auto it = cache.find(key);
  if (it != cache.end()) return it->second;

  const int minutes = oracle_.minutes(from, to);
  cache.emplace(key, minutes);
  return minutes;
}

Plan Planner::plan(const std::vector<Van>& vans, const std::vector<Parcel>& parcels) const {
  struct VanState {
    const Van* van;
    int clock;
    std::string position;
  };

  Plan result;
  std::vector<VanState> states;
  for (const Van& van : vans) {
    states.push_back({&van, van.shiftStart, van.depot});
    result.vans.push_back({van.id, van.capacity, 0, {}});
  }

  DispatchQueue queue;
  for (const Parcel& parcel : parcels) {
    if (!parcel.cancelled) queue.push(parcel);
  }

  while (!queue.empty()) {
    const Parcel parcel = queue.pop();
    std::string reason = vans.empty() ? "no vans" : "no van has capacity left";
    bool assigned = false;

    for (std::size_t i = 0; i < states.size() && !assigned; ++i) {
      VanState& state = states[i];
      VanPlan& plan = result.vans[i];

      if (plan.load + parcel.size > state.van->capacity) continue;

      const int travel = travelMinutes(state.position, parcel.destination);
      if (travel < 0) {
        reason = "destination unreachable";
        continue;
      }

      const int arrival = state.clock + travel;
      if (arrival + serviceMinutes_ > state.van->shiftEnd) {
        reason = "no van can deliver before its shift ends";
        continue;
      }

      plan.load += parcel.size;
      plan.stops.push_back({parcel.id, parcel.destination, arrival, arrival > parcel.deadline});
      state.clock = arrival + serviceMinutes_;
      state.position = parcel.destination;
      assigned = true;
    }

    if (!assigned) result.unassigned.push_back({parcel.id, reason});
  }

  return result;
}

}  // namespace vanroute
