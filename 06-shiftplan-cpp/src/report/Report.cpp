#include "report/Report.hpp"

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <unordered_map>

#include "io/Clock.hpp"
#include "plan/RushBoost.hpp"
#include "plan/Scheduler.hpp"
#include "report/Departures.hpp"
#include "report/Metrics.hpp"

namespace shiftplan {

namespace {

std::string join(const std::vector<std::string>& ids, const std::string& separator) {
  if (ids.empty()) return "none";
  std::string out;
  for (std::size_t i = 0; i < ids.size(); ++i) out += (i ? separator : "") + ids[i];
  return out;
}

std::string percent(double value) {
  char buffer[32];
  std::snprintf(buffer, sizeof buffer, "%.1f%%", value);
  return buffer;
}

std::string pad(const std::string& text, std::size_t width) {
  return text.size() >= width ? text : text + std::string(width - text.size(), ' ');
}

}  // namespace

std::string renderReport(const SiteData& site) {
  const Shift& shift = site.shift;
  const RushBoost rush = applyRushBoost(site.jobs, site.trucks, shift);
  const std::vector<Job>& jobs = rush.jobs;
  std::unordered_map<std::string, const Job*> byId;
  for (const Job& job : jobs) byId.emplace(job.id, &job);

  std::ostringstream out;
  out << "ShiftPlan outbound report\n";
  out << "Site: " << shift.site << ", shift " << formatClock(shift.start) << "-" << shift.clockAt(shift.minutes) << " ("
      << shift.minutes << " min)\n";
  out << "Crews:";
  int totalCrews = 0;
  for (const auto& [type, count] : site.crews) {
    out << " " << type << " " << count;
    totalCrews += count;
  }
  out << "\nJobs: " << jobs.size() << ", trucks: " << site.trucks.size() << "\n";

  out << "\n== Rush trucks (PRI-2) ==\n";
  out << "Rush trucks: " << join(rush.rushTrucks, ", ") << "\n";
  out << "Boosted jobs (+" << kRushBoost << "): " << join(rush.boosted, ", ") << "\n";

  out << "\n== Dispatch board ==\n";
  const auto order = dispatchOrder(jobs);
  for (std::size_t i = 0; i < order.size(); ++i) {
    const Job& job = *byId.at(order[i]);
    out << pad(std::to_string(i + 1) + ".", 4) << pad(job.id, 6) << pad(job.crew, 10) << "priority " << job.priority
        << "\n";
  }

  out << "\n== Critical path (unlimited crews) ==\n";
  const auto path = criticalPath(jobs);
  if (path.empty()) {
    out << "n/a\n";
  } else {
    int end = 0;
    for (const Slot& slot : earliestTimes(jobs)) {
      if (slot.job == path.back()) end = slot.finish;
    }
    out << join(path, " -> ") << ": all work done by " << shift.clockAt(end) << "\n";
  }

  out << "\n== What-if: fully cross-trained crews ==\n";
  const auto pooled = scheduleCrews(jobs, totalCrews);
  if (pooled.empty()) {
    out << "n/a\n";
  } else {
    int end = 0;
    for (const Slot& slot : pooled) end = std::max(end, slot.finish);
    out << "If all " << totalCrews << " crews could do every job, all work would be done by " << shift.clockAt(end)
        << "\n";
  }

  const ShiftPlan plan = planShift(jobs, site.crews, shift.minutes);
  out << "\n== Crew timeline ==\n";
  for (const Slot& slot : plan.slots) {
    const Job& job = *byId.at(slot.job);
    out << shift.clockAt(slot.start) << "-" << shift.clockAt(slot.finish) << "  "
        << pad(job.crew + " #" + std::to_string(slot.crew), 13) << job.id << "\n";
  }
  out << "Carried over to the next shift: " << join(plan.carriedOver, ", ") << "\n";

  out << "\n== Departure board ==\n";
  const auto board = truckStatuses(site.trucks, plan, shift);
  for (std::size_t i = 0; i < board.size(); ++i) {
    const TruckStatus& s = board[i];
    out << pad(s.truck, 5) << "cutoff " << formatClock(site.trucks[i].cutoff) << "  ";
    switch (s.state) {
      case TruckState::OnTime:
        out << "sealed " << shift.clockAt(*s.sealedAt) << "  ON TIME";
        break;
      case TruckState::Late:
        out << "sealed " << shift.clockAt(*s.sealedAt) << "  LATE";
        break;
      case TruckState::CarriedOver:
        out << "CARRIED OVER (" << s.loadJob << " not run this shift)";
        break;
    }
    out << "\n";
  }

  out << "\n== Summary ==\n";
  out << "Jobs run this shift: " << plan.slots.size() << " of " << jobs.size() << "\n";
  out << "Last job finishes:   " << shift.clockAt(lastFinish(plan)) << "\n";
  out << "Trucks on time:      " << trucksOnTime(board) << " of " << board.size() << "\n";
  out << "Team utilization:   ";
  for (const auto& [type, value] : teamUtilizationPercent(plan, jobs, site.crews, shift.minutes)) {
    out << " " << type << " " << percent(value);
  }
  out << "\nCrew utilization:    " << percent(crewUtilizationPercent(plan, jobs, site.crews, shift.minutes)) << "\n";
  return out.str();
}

std::string runReport(const std::string& dataDir) {
  return renderReport(loadSite(dataDir));
}

}  // namespace shiftplan
