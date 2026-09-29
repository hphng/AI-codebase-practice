#pragma once

#include <cstdint>
#include <vector>

#include "graph/RoadGraph.hpp"

namespace vanroute {

// PART A: implement shortestTimes (src/graph/ShortestPath.cpp).
//
// Returns the minimum total travel time, in minutes, from `source` to every node of `graph`.
// The result has graph.nodeCount() entries, indexed by node number:
//   - result[source] == 0
//   - result[v] == -1 if v can't be reached from source
//
// Details:
//   - Roads are two-way (RoadGraph stores both directions) with integer minutes >= 0.
//     Zero-minute roads are allowed. Several roads may connect the same two nodes.
//   - Sums can exceed the int range on large maps; use std::int64_t.
//   - Throws std::out_of_range if `source` is not a valid node index.
//
// Performance: up to 200,000 nodes and 600,000 roads. There is a performance test.
std::vector<std::int64_t> shortestTimes(const RoadGraph& graph, int source);

}  // namespace vanroute
