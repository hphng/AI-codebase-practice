#include "report/Metrics.hpp"

#include <algorithm>
#include <unordered_map>

namespace shiftplan {

std::map<std::string, long long> minutesWorkedByType(const ShiftPlan& plan, const std::vector<Job>& jobs) {
  std::unordered_map<std::string, const Job*> byId;
  for (const Job& job : jobs) byId.emplace(job.id, &job);

  std::map<std::string, long long> worked;
  for (const Slot& slot : plan.slots) {
    const auto it = byId.find(slot.job);
    if (it == byId.end()) continue;
    worked[it->second->crew] += slot.finish - slot.start;
  }
  return worked;
}

std::map<std::string, double> teamUtilizationPercent(const ShiftPlan& plan, const std::vector<Job>& jobs,
                                                     const CrewPool& crews, int shiftMinutes) {
  const auto worked = minutesWorkedByType(plan, jobs);
  std::map<std::string, double> teams;
  for (const auto& [type, count] : crews) {
    const auto it = worked.find(type);
    const double minutes = it == worked.end() ? 0.0 : static_cast<double>(it->second);
    teams[type] = 100.0 * minutes / (static_cast<double>(count) * shiftMinutes);
  }
  return teams;
}

double crewUtilizationPercent(const ShiftPlan& plan, const std::vector<Job>& jobs, const CrewPool& crews,
                              int shiftMinutes) {
  const auto teams = teamUtilizationPercent(plan, jobs, crews, shiftMinutes);
  if (teams.empty()) return 0.0;
  // The site figure is the average of the team figures shown in the report, so the two always agree.
  double sum = 0.0;
  for (const auto& [type, value] : teams) sum += value;
  return sum / static_cast<double>(teams.size());
}

int trucksOnTime(const std::vector<TruckStatus>& board) {
  return static_cast<int>(
      std::count_if(board.begin(), board.end(), [](const TruckStatus& s) { return s.state == TruckState::OnTime; }));
}

int lastFinish(const ShiftPlan& plan) {
  int last = 0;
  for (const Slot& slot : plan.slots) last = std::max(last, slot.finish);
  return last;
}

}  // namespace shiftplan
