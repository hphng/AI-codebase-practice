#pragma once

// Large pseudo-random road network for the Part A performance test:
// 200,000 nodes, a spanning chain (so everything is reachable) plus 400,000 random roads.

#include <cstdint>
#include <string>

#include "graph/RoadGraph.hpp"

inline vanroute::RoadGraph makeBigNetwork() {
  const int nodes = 200000;
  const int extraRoads = 400000;
  std::uint64_t seed = 42;
  const auto next = [&seed]() {
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return static_cast<std::uint32_t>(seed >> 33);
  };

  vanroute::RoadGraph g;
  for (int i = 0; i < nodes; ++i) g.addNode("N" + std::to_string(i));
  for (int i = 0; i + 1 < nodes; ++i) {
    g.addRoad("N" + std::to_string(i), "N" + std::to_string(i + 1), static_cast<int>(1 + next() % 1000));
  }
  for (int k = 0; k < extraRoads; ++k) {
    const int a = static_cast<int>(next() % nodes);
    const int b = static_cast<int>(next() % nodes);
    g.addRoad("N" + std::to_string(a), "N" + std::to_string(b), static_cast<int>(next() % 5000));
  }
  return g;
}
