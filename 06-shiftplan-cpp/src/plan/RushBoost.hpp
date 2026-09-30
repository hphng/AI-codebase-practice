#pragma once

#include <string>
#include <vector>

#include "model/Job.hpp"
#include "model/Shift.hpp"
#include "model/Truck.hpp"

namespace shiftplan {

// Ops rule PRI-2 (rush trucks). A truck whose cutoff is less than kRushWindowMinutes after the shift
// starts is a rush truck. Every job that has to be finished before a rush truck can leave gets
// kRushBoost extra priority: its load job, the load job's prerequisites, their prerequisites, and so
// on, all the way back to the unloads. A job that feeds several rush trucks is boosted only once.
constexpr int kRushWindowMinutes = 180;
constexpr int kRushBoost = 100;

struct RushBoost {
  std::vector<Job> jobs;                // copy of the input, same order, with boosted priorities
  std::vector<std::string> rushTrucks;  // ids of the rush trucks, in input order
  std::vector<std::string> boosted;     // ids of the boosted jobs, sorted
};

// Applies PRI-2 before planning. Prerequisite ids that aren't jobs are ignored here (the planner
// reports them).
RushBoost applyRushBoost(const std::vector<Job>& jobs, const std::vector<Truck>& trucks, const Shift& shift);

}  // namespace shiftplan
