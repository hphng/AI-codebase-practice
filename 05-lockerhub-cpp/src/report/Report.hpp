#pragma once

#include <string>

#include "model/StationConfig.hpp"
#include "station/StationDay.hpp"

namespace lockerhub {

// The station's daily report: the activity log, the state of the lockers at close, and the summary
// figures (counts, pickup rate, dwell time, partner payout, reminders due at close).
std::string renderReport(const StationConfig& station, int compartmentCount, const DayLog& log);

// Loads station.csv, lockers.csv and events.csv from `dataDir`, runs the day and renders the report.
std::string runReport(const std::string& dataDir);

}  // namespace lockerhub
