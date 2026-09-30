#pragma once

#include <string>
#include <vector>

#include "model/Job.hpp"
#include "model/Shift.hpp"
#include "model/Truck.hpp"

namespace shiftplan {

// File formats (CSV with a header row; see data/ for examples):
//
//   shift.csv   site,start,minutes              one row; start is HH:MM, minutes > 0
//   crews.csv   crew,count                      one row per crew type; count > 0
//   jobs.csv    job,minutes,priority,crew,after after = prerequisite ids separated by ';' (may be empty)
//   trucks.csv  truck,load_job,cutoff           load_job must be a job in jobs.csv; cutoff is HH:MM
//
// The loaders check each file's own format and throw std::runtime_error naming the row. Whether the
// jobs make a valid plan (known prerequisites, no cycles, ...) is the planner's job, not theirs.

using Rows = std::vector<std::vector<std::string>>;

Shift shiftFromRows(const Rows& rows, const std::string& source);
CrewPool crewsFromRows(const Rows& rows, const std::string& source);
std::vector<Job> jobsFromRows(const Rows& rows, const std::string& source);
std::vector<Truck> trucksFromRows(const Rows& rows, const std::string& source);

struct SiteData {
  Shift shift;
  CrewPool crews;
  std::vector<Job> jobs;
  std::vector<Truck> trucks;
};

// Loads the four files from `dir` and checks that every truck's load job exists.
SiteData loadSite(const std::string& dir);

}  // namespace shiftplan
