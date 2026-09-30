#include "report/Report.hpp"

#include <cstdio>
#include <sstream>

#include "billing/Payout.hpp"
#include "io/Clock.hpp"
#include "io/Loaders.hpp"
#include "notify/Reminders.hpp"
#include "report/Metrics.hpp"

namespace lockerhub {

namespace {

std::string joinOrNone(const std::vector<std::string>& ids) {
  if (ids.empty()) return "none";
  std::string out;
  for (std::size_t i = 0; i < ids.size(); ++i) out += (i ? ", " : "") + ids[i];
  return out;
}

int countState(const std::vector<ParcelRecord>& records, ParcelState state) {
  int n = 0;
  for (const ParcelRecord& r : records) n += r.state == state ? 1 : 0;
  return n;
}

std::string percent(double value) {
  char buffer[32];
  std::snprintf(buffer, sizeof buffer, "%.1f%%", value);
  return buffer;
}

}  // namespace

std::string renderReport(const StationConfig& station, int compartmentCount, const DayLog& log) {
  std::ostringstream out;
  out << "LockerHub station report\n";
  out << "Station: " << station.name << " (" << station.id << "), " << compartmentCount
      << " compartments, hold " << station.holdHours << "h\n";

  out << "\n== Activity ==\n";
  for (const std::string& line : log.activity) out << line << "\n";

  out << "\n== Lockers at close (" << formatStamp(log.closedAt) << ") ==\n";
  out << "Empty compartments: S " << log.freeAtClose[0] << ", M " << log.freeAtClose[1] << ", L "
      << log.freeAtClose[2] << "\n";
  out << "Waitlist: " << joinOrNone(log.waitlistAtClose) << "\n";
  out << "In a locker longest:";
  if (log.longestInLockers.empty()) out << " none";
  for (std::size_t i = 0; i < log.longestInLockers.size(); ++i) {
    const std::string& id = log.longestInLockers[i];
    std::string since = "?";
    for (const ParcelRecord& r : log.records) {
      if (r.id == id && r.state == ParcelState::InLocker && r.placedAt) since = formatStamp(*r.placedAt);
    }
    out << (i ? ", " : " ") << id << " (since " << since << ")";
  }
  out << "\n";

  const auto& records = log.records;
  out << "\n== Summary ==\n";
  out << "Parcels received:   " << records.size() << "\n";
  out << "Picked up:          " << countState(records, ParcelState::PickedUp) << "\n";
  out << "Returned to sender: " << countState(records, ParcelState::Returned) << "\n";
  out << "Still in lockers:   " << countState(records, ParcelState::InLocker) << "\n";
  out << "On the waitlist:    " << countState(records, ParcelState::Waiting) << "\n";
  out << "Pickup rate:        " << percent(pickupRatePercent(records)) << "\n";
  const auto dwell = averageDwellMinutes(records);
  out << "Average dwell:      " << (dwell ? formatDuration(*dwell) : std::string("n/a")) << "\n";
  out << "Partner payout:     " << formatMoney(partnerPayoutCents(records)) << "\n";
  out << "Reminders due:      " << joinOrNone(dueReminders(records, log.closedAt)) << "\n";
  return out.str();
}

std::string runReport(const std::string& dataDir) {
  const StationConfig station = loadStation(dataDir + "/station.csv");
  const auto compartments = loadCompartments(dataDir + "/lockers.csv");
  const auto events = loadEvents(dataDir + "/events.csv");
  const DayLog log = runDay(station, compartments, events);
  return renderReport(station, static_cast<int>(compartments.size()), log);
}

}  // namespace lockerhub
