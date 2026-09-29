#include <chrono>
#include <stdexcept>
#include <vector>

#include "dispatch/TravelOracle.hpp"
#include "fixtures/BigNetwork.hpp"
#include "graph/ShortestPath.hpp"
#include "test_framework.hpp"

using namespace vanroute;

namespace {

std::int64_t timeTo(const RoadGraph& g, const std::string& from, const std::string& to) {
  return shortestTimes(g, g.indexOf(from))[static_cast<std::size_t>(g.indexOf(to))];
}

}  // namespace

TEST(ShortestTimes, SumsAlongASingleRoute) {
  RoadGraph g;
  g.addRoad("A", "B", 5);
  g.addRoad("B", "C", 7);
  EXPECT_EQ(shortestTimes(g, g.indexOf("A")), (std::vector<std::int64_t>{0, 5, 12}));
  EXPECT_EQ(timeTo(g, "C", "A"), 12);  // roads are two-way
}

TEST(ShortestTimes, PrefersTheFasterRouteEvenWithMoreRoads) {
  RoadGraph g;
  g.addRoad("A", "B", 10);  // direct but slow
  g.addRoad("A", "C", 2);
  g.addRoad("C", "D", 2);
  g.addRoad("D", "B", 2);
  EXPECT_EQ(timeTo(g, "A", "B"), 6);
}

TEST(ShortestTimes, UnreachableNodesAreMinusOne) {
  RoadGraph g;
  g.addRoad("A", "B", 3);
  g.addRoad("ISLAND", "LIGHTHOUSE", 4);
  const auto t = shortestTimes(g, g.indexOf("A"));
  EXPECT_EQ(t[static_cast<std::size_t>(g.indexOf("ISLAND"))], -1);
  EXPECT_EQ(t[static_cast<std::size_t>(g.indexOf("LIGHTHOUSE"))], -1);
  EXPECT_EQ(t[static_cast<std::size_t>(g.indexOf("B"))], 3);
}

TEST(ShortestTimes, ZeroMinuteRoadsAndParallelRoads) {
  RoadGraph g;
  g.addRoad("A", "B", 0);
  g.addRoad("B", "C", 9);
  g.addRoad("B", "C", 4);  // a faster parallel road
  g.addRoad("C", "D", 0);
  EXPECT_EQ(shortestTimes(g, g.indexOf("A")), (std::vector<std::int64_t>{0, 0, 4, 4}));
}

TEST(ShortestTimes, LateRelaxationBeatsAnEarlierGuess) {
  // T is first reached by a slow direct road, then improved via a chain of fast roads.
  RoadGraph g;
  g.addRoad("S", "T", 100);
  g.addRoad("S", "P", 1);
  g.addRoad("P", "Q", 1);
  g.addRoad("Q", "R", 1);
  g.addRoad("R", "T", 1);
  g.addRoad("T", "U", 1);
  EXPECT_EQ(timeTo(g, "S", "T"), 4);
  EXPECT_EQ(timeTo(g, "S", "U"), 5);
}

TEST(ShortestTimes, TotalsBeyondIntRange) {
  RoadGraph g;
  g.addRoad("A", "B", 2000000000);
  g.addRoad("B", "C", 2000000000);
  g.addRoad("C", "D", 2000000000);
  EXPECT_EQ(timeTo(g, "A", "D"), 6000000000LL);
}

TEST(ShortestTimes, RejectsABadSource) {
  RoadGraph g;
  g.addRoad("A", "B", 1);
  EXPECT_THROW(shortestTimes(g, -1), std::out_of_range);
  EXPECT_THROW(shortestTimes(g, 2), std::out_of_range);
  EXPECT_EQ(shortestTimes(g, 1), (std::vector<std::int64_t>{1, 0}));
}

TEST(ShortestTimes, RoadNetworkOracleAnswersPlannerQueries) {
  RoadGraph g;
  g.addRoad("DEPOT", "N1", 5);
  g.addRoad("N1", "N2", 3);
  g.addRoad("DEPOT", "N2", 9);
  const RoadNetworkOracle oracle(g);
  EXPECT_EQ(oracle.minutes("DEPOT", "N2"), 8);
  EXPECT_EQ(oracle.minutes("N2", "DEPOT"), 8);
  EXPECT_EQ(oracle.minutes("DEPOT", "NOWHERE"), -1);
}

TEST(ShortestTimes, LargeNetworkIsFast) {
  const RoadGraph g = makeBigNetwork();

  const auto started = std::chrono::steady_clock::now();
  const auto t = shortestTimes(g, 0);
  const auto ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count();

  std::int64_t sum = 0;
  std::int64_t reachable = 0;
  for (std::int64_t x : t) {
    if (x >= 0) {
      sum += x;
      ++reachable;
    }
  }
  EXPECT_EQ(reachable, 200000);
  EXPECT_EQ(sum, 1273599116LL);
  EXPECT_EQ(t[199999], 7659);
  EXPECT_EQ(t[12345], 6736);
  EXPECT_TRUE(ms < 3000);
}
