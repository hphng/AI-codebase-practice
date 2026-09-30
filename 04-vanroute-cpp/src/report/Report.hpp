#pragma once

#include <string>

#include "dispatch/Planner.hpp"

namespace vanroute {

// How much of the fleet's capacity went out on the road today: the average of (load / capacity)
// over ALL vans on the plan, in [0, 1]. A van that never left the depot counts as 0%, because it's
// capacity we paid for and didn't use. 0 when there are no vans.
// Example: vans at 6/6, 2/4 and an idle 0/4 -> (1.0 + 0.5 + 0.0) / 3 = 0.5.
double fleetUtilization(const Plan& plan);

// Number of stops that arrive after their parcel's deadline.
int lateCount(const Plan& plan);

// Human-readable dispatch report (see data/expected_report.txt).
std::string renderReport(const Plan& plan);

// Loads roads.csv, vans.csv and parcels.csv from `dataDir`, plans the day and renders the report.
std::string runReport(const std::string& dataDir);

}  // namespace vanroute
