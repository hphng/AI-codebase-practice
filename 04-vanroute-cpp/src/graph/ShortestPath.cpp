#include "graph/ShortestPath.hpp"

#include <stdexcept>

namespace vanroute {

std::vector<std::int64_t> shortestTimes(const RoadGraph& graph, int source) {
  if (source < 0 || static_cast<std::size_t>(source) >= graph.nodeCount()) {
    throw std::out_of_range("shortestTimes: bad source node");
  }

  // TODO(Part A): replace this placeholder. Right now only the source itself is
  // "reachable", so the planner can't route any van anywhere.
  std::vector<std::int64_t> times(graph.nodeCount(), -1);
  times[static_cast<std::size_t>(source)] = 0;
  return times;
}

}  // namespace vanroute
