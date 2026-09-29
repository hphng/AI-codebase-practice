#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace vanroute {

struct Edge {
  int to = 0;
  int minutes = 0;  // travel time, >= 0
};

// Road network. Nodes are named ("DEPOT", "N07", ...) and numbered 0..nodeCount()-1 in the order
// they were first seen. Every road is two-way. Parallel roads between the same nodes are allowed.
class RoadGraph {
 public:
  int addNode(const std::string& name);  // returns the existing index if already present
  void addRoad(const std::string& a, const std::string& b, int minutes);  // throws if minutes < 0

  int indexOf(const std::string& name) const;  // -1 if unknown
  const std::string& nameOf(int index) const;
  std::size_t nodeCount() const { return names_.size(); }
  const std::vector<Edge>& neighbors(int node) const { return adjacency_.at(static_cast<std::size_t>(node)); }

 private:
  std::vector<std::string> names_;
  std::unordered_map<std::string, int> index_;
  std::vector<std::vector<Edge>> adjacency_;
};

}  // namespace vanroute
