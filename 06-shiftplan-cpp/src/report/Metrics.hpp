#pragma once

#include <map>
#include <string>
#include <vector>

#include "model/Job.hpp"
#include "plan/Scheduler.hpp"
#include "report/Departures.hpp"

namespace shiftplan {

// Minutes worked per crew type: the total length of the slots of that type's jobs. Types that did
// nothing are absent.
std::map<std::string, long long> minutesWorkedByType(const ShiftPlan& plan, const std::vector<Job>& jobs);

// Utilization of each team (crew type) in percent: minutes it worked / (its crews x shift length).
// Every type in `crews` is listed, idle ones at 0.0.
std::map<std::string, double> teamUtilizationPercent(const ShiftPlan& plan, const std::vector<Job>& jobs,
                                                     const CrewPool& crews, int shiftMinutes);

// Crew utilization for the whole site (ops metric OPS-11): the share of paid crew time spent working.
//
//     minutes worked by all crews together / (number of crews on shift x shift length)
//
// It is pooled over every crew on shift, so a team of 6 crews weighs three times as much as a team
// of 2, and a team with nothing to do still counts as paid time. Carried-over jobs didn't run, so they
// add nothing. 0.0 if there are no crews.
double crewUtilizationPercent(const ShiftPlan& plan, const std::vector<Job>& jobs, const CrewPool& crews,
                              int shiftMinutes);

// Number of trucks on the board that are on time.
int trucksOnTime(const std::vector<TruckStatus>& board);

// When the last job of the plan finishes (minutes since shift start), or 0 for an empty plan.
int lastFinish(const ShiftPlan& plan);

}  // namespace shiftplan
