#pragma once

#include <string>

#include "dispatch/Planner.hpp"

namespace vanroute {

// Average of (load / capacity) over all vans, in [0, 1]. 0 when there are no vans.
// Example: vans at 6/6 and 2/4 -> (1.0 + 0.5) / 2 = 0.75.
double fleetUtilization(const Plan& plan);

// Number of stops that arrive after their parcel's deadline.
int lateCount(const Plan& plan);

// Human-readable dispatch report (see data/expected_report.txt).
std::string renderReport(const Plan& plan);

// Loads roads.csv, vans.csv and parcels.csv from `dataDir`, plans the day and renders the report.
std::string runReport(const std::string& dataDir);

}  // namespace vanroute
