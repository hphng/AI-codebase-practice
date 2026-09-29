#include "report/Report.hpp"

#include <iomanip>
#include <numeric>
#include <sstream>
#include <vector>

#include "dispatch/TravelOracle.hpp"
#include "io/Csv.hpp"
#include "io/Loaders.hpp"

namespace vanroute {

double fleetUtilization(const Plan& plan) {
  if (plan.vans.empty()) return 0.0;

  std::vector<double> ratios;
  for (const VanPlan& van : plan.vans) ratios.push_back(static_cast<double>(van.load) / van.capacity);
  return std::accumulate(ratios.begin(), ratios.end(), 0) / ratios.size();
}

int lateCount(const Plan& plan) {
  int late = 0;
  for (const VanPlan& van : plan.vans) {
    for (const Stop& stop : van.stops) late += stop.late ? 1 : 0;
  }
  return late;
}

std::string renderReport(const Plan& plan) {
  std::ostringstream out;

  for (const VanPlan& van : plan.vans) {
    out << "Van " << van.vanId << "  load " << van.load << "/" << van.capacity << "\n";
    if (van.stops.empty()) out << "  (no stops)\n";
    for (const Stop& stop : van.stops) {
      out << "  " << formatClock(stop.arrival) << "  " << std::left << std::setw(5) << stop.parcelId << " -> "
          << std::setw(10) << stop.destination << (stop.late ? "LATE" : "") << "\n";
    }
  }

  out << "\nUnassigned (" << plan.unassigned.size() << "):\n";
  for (const Unassigned& u : plan.unassigned) out << "  " << std::left << std::setw(5) << u.parcelId << " " << u.reason << "\n";

  out << "\nLate deliveries: " << lateCount(plan) << "\n";
  out << "Fleet utilization: " << std::fixed << std::setprecision(1) << fleetUtilization(plan) * 100.0 << "%\n";
  return out.str();
}

std::string runReport(const std::string& dataDir) {
  const auto path = [&dataDir](const char* file) { return dataDir + "/" + file; };

  const RoadGraph graph = loadRoads(readCsv(path("roads.csv")));
  const std::vector<Van> vans = loadVans(readCsv(path("vans.csv")));
  const std::vector<Parcel> parcels = loadParcels(readCsv(path("parcels.csv")));

  const RoadNetworkOracle oracle(graph);
  const Planner planner(oracle);
  return renderReport(planner.plan(vans, parcels));
}

}  // namespace vanroute
