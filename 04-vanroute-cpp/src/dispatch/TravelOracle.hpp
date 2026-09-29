#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "graph/RoadGraph.hpp"

namespace vanroute {

// Answers "how many minutes from node A to node B?" (-1 if there is no route).
class TravelOracle {
 public:
  virtual ~TravelOracle() = default;
  virtual int minutes(const std::string& from, const std::string& to) const = 0;
};

// Production oracle: shortest travel times over the road network (see graph/ShortestPath.hpp).
// Results are cached per source node.
class RoadNetworkOracle : public TravelOracle {
 public:
  explicit RoadNetworkOracle(const RoadGraph& graph) : graph_(graph) {}
  int minutes(const std::string& from, const std::string& to) const override;

 private:
  const RoadGraph& graph_;
  mutable std::unordered_map<int, std::vector<std::int64_t>> bySource_;
};

}  // namespace vanroute
