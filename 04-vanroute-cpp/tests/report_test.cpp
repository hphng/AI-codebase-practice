#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

#include "report/Report.hpp"
#include "test_framework.hpp"

using namespace vanroute;

namespace {

// (load, capacity) per van. Vans with a load get one stop per unit; empty vans stay at the depot.
Plan planWith(std::vector<std::pair<int, int>> loads) {
  Plan plan;
  int n = 0;
  for (auto [load, capacity] : loads) {
    VanPlan van{"V" + std::to_string(++n), capacity, load, {}};
    for (int i = 0; i < load; ++i) van.stops.push_back({"P" + std::to_string(i), "X", 480 + i, false});
    plan.vans.push_back(van);
  }
  return plan;
}

}  // namespace

TEST(Report, FleetUtilizationAveragesVans) {
  EXPECT_TRUE(std::fabs(fleetUtilization(planWith({{6, 6}, {2, 4}})) - 0.75) < 1e-9);
  EXPECT_TRUE(std::fabs(fleetUtilization(planWith({{3, 4}, {1, 2}})) - 0.625) < 1e-9);
  EXPECT_TRUE(std::fabs(fleetUtilization(planWith({{1, 3}})) - 1.0 / 3.0) < 1e-9);
}

TEST(Report, FleetUtilizationCountsIdleVans) {
  EXPECT_TRUE(std::fabs(fleetUtilization(planWith({{6, 6}, {2, 4}, {0, 4}})) - 0.5) < 1e-9);
  EXPECT_TRUE(std::fabs(fleetUtilization(planWith({{4, 4}, {0, 4}})) - 0.5) < 1e-9);
}

TEST(Report, FleetUtilizationEdgeCases) {
  EXPECT_EQ(fleetUtilization(planWith({})), 0.0);
  EXPECT_EQ(fleetUtilization(planWith({{0, 5}, {0, 2}})), 0.0);
  EXPECT_EQ(fleetUtilization(planWith({{5, 5}, {2, 2}})), 1.0);
}

TEST(Report, CountsLateStops) {
  Plan plan = planWith({{2, 4}});
  plan.vans[0].stops = {{"P1", "A", 500, false}, {"P2", "B", 530, true}, {"P3", "C", 560, true}};
  EXPECT_EQ(lateCount(plan), 2);
}

TEST(Report, RendersStopsAndTotals) {
  Plan plan = planWith({{3, 4}});
  plan.vans[0].stops = {{"P1", "N04", 515, true}};
  plan.unassigned = {{"P2", "destination unreachable"}};
  const std::string text = renderReport(plan);
  EXPECT_CONTAINS(text, "Van V1  load 3/4");
  EXPECT_CONTAINS(text, "08:35  P1    -> N04       LATE");
  EXPECT_CONTAINS(text, "P2    destination unreachable");
  EXPECT_CONTAINS(text, "Fleet utilization: 75.0%");
}

// ---------- End to end: data/ directory vs. data/expected_report.txt ----------

TEST(SampleData, ReportMatchesExpectedOutput) {
  std::ifstream in(std::string(SAMPLE_DATA_DIR) + "/expected_report.txt", std::ios::binary);
  ASSERT_TRUE(in.good());
  std::ostringstream buffer;
  buffer << in.rdbuf();
  std::string expected = buffer.str();
  expected.erase(std::remove(expected.begin(), expected.end(), '\r'), expected.end());

  std::string actual;
  EXPECT_NO_THROW(actual = runReport(SAMPLE_DATA_DIR));
  EXPECT_EQ(actual, expected);
}
