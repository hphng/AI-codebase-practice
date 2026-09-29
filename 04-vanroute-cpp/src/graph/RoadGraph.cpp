#include "graph/RoadGraph.hpp"

#include <stdexcept>

namespace vanroute {

int RoadGraph::addNode(const std::string& name) {
  auto it = index_.find(name);
  if (it != index_.end()) return it->second;
  const int id = static_cast<int>(names_.size());
  names_.push_back(name);
  index_.emplace(name, id);
  adjacency_.emplace_back();
  return id;
}

void RoadGraph::addRoad(const std::string& a, const std::string& b, int minutes) {
  if (minutes < 0) throw std::invalid_argument("road " + a + "-" + b + " has negative travel time");
  const int from = addNode(a);
  const int to = addNode(b);
  adjacency_[static_cast<std::size_t>(from)].push_back({to, minutes});
  adjacency_[static_cast<std::size_t>(to)].push_back({from, minutes});
}

int RoadGraph::indexOf(const std::string& name) const {
  auto it = index_.find(name);
  return it == index_.end() ? -1 : it->second;
}

const std::string& RoadGraph::nameOf(int index) const {
  return names_.at(static_cast<std::size_t>(index));
}

}  // namespace vanroute
