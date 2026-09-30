#include "plan/Scheduler.hpp"

namespace shiftplan {

// TODO(Part A): replace these placeholders with a real implementation of the spec in Scheduler.hpp.
// Right now nothing is planned: the dispatch board is just the file order, nothing is ever blocked,
// every job "starts" at 0, there is no critical path, the crews get no work, and planShift carries
// every job over to the next shift.

std::vector<std::string> dispatchOrder(const std::vector<Job>& jobs) {
  std::vector<std::string> ids;
  for (const Job& job : jobs) ids.push_back(job.id);
  return ids;
}

std::vector<std::string> blockedJobs(const std::vector<Job>& /*jobs*/) {
  return {};
}

std::vector<Slot> earliestTimes(const std::vector<Job>& jobs) {
  std::vector<Slot> slots;
  for (const Job& job : jobs) slots.push_back(Slot{job.id, 0, job.minutes, 0});
  return slots;
}

std::vector<std::string> criticalPath(const std::vector<Job>& /*jobs*/) {
  return {};
}

std::vector<Slot> scheduleCrews(const std::vector<Job>& /*jobs*/, int /*crews*/) {
  return {};
}

ShiftPlan planShift(const std::vector<Job>& jobs, const CrewPool& /*crews*/, int /*shiftMinutes*/) {
  ShiftPlan plan;
  for (const Job& job : jobs) plan.carriedOver.push_back(job.id);
  return plan;
}

}  // namespace shiftplan
