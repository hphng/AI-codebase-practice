#include "dispatch/TravelOracle.hpp"

#include "graph/ShortestPath.hpp"

namespace vanroute {

int RoadNetworkOracle::minutes(const std::string& from, const std::string& to) const {
  const int a = graph_.indexOf(from);
  const int b = graph_.indexOf(to);
  if (a < 0 || b < 0) return -1;

  auto it = bySource_.find(a);
  if (it == bySource_.end()) it = bySource_.emplace(a, shortestTimes(graph_, a)).first;
  return static_cast<int>(it->second[static_cast<std::size_t>(b)]);
}

}  // namespace vanroute
