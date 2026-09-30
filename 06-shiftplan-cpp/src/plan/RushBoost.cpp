#include "plan/RushBoost.hpp"

#include <algorithm>
#include <set>
#include <unordered_map>

namespace shiftplan {

RushBoost applyRushBoost(const std::vector<Job>& jobs, const std::vector<Truck>& trucks, const Shift& shift) {
  RushBoost result;
  result.jobs = jobs;

  std::unordered_map<std::string, std::size_t> byId;
  for (std::size_t i = 0; i < jobs.size(); ++i) byId.emplace(jobs[i].id, i);

  std::set<std::string> boosted;
  for (const Truck& truck : trucks) {
    if (shift.minutesIntoShift(truck.cutoff) >= kRushWindowMinutes) continue;
    result.rushTrucks.push_back(truck.id);

    // The load job itself, plus the sort jobs it waits for.
    const auto load = byId.find(truck.loadJob);
    if (load == byId.end()) continue;
    boosted.insert(truck.loadJob);
    for (const std::string& dep : jobs[load->second].after) {
      if (byId.count(dep)) boosted.insert(dep);
    }
  }

  for (const std::string& id : boosted) result.jobs[byId.at(id)].priority += kRushBoost;
  result.boosted.assign(boosted.begin(), boosted.end());
  return result;
}

}  // namespace shiftplan
