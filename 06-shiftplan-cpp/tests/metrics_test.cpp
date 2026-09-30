#include "JobBuilders.hpp"
#include "report/Metrics.hpp"
#include "test_framework.hpp"

using namespace shiftplan;
using namespace testjobs;

namespace {

// Two loads (loader) and one sort (sorter).
const std::vector<Job> kJobs = {
    job("L1", 50, 1, {}, "loader"), job("L2", 30, 1, {}, "loader"), job("S1", 100, 1, {}, "sorter"),
};

ShiftPlan planOf(const std::vector<Slot>& slots) {
  ShiftPlan plan;
  plan.slots = slots;
  return plan;
}

}  // namespace

TEST(Metrics, MinutesWorkedPerCrewType) {
  const auto worked = minutesWorkedByType(planOf({{"L1", 0, 50, 1}, {"L2", 0, 30, 2}, {"S1", 0, 100, 1}}), kJobs);
  EXPECT_EQ(worked.at("loader"), 80);
  EXPECT_EQ(worked.at("sorter"), 100);
  EXPECT_EQ(worked.count("unloader"), 0u);
}

TEST(Metrics, TeamUtilizationListsEveryTeam) {
  const auto teams = teamUtilizationPercent(planOf({{"L1", 0, 50, 1}, {"L2", 0, 30, 2}}), kJobs,
                                            {{"loader", 2}, {"sorter", 1}}, 100);
  EXPECT_EQ(teams.at("loader"), 40.0);  // 80 of 2 x 100
  EXPECT_EQ(teams.at("sorter"), 0.0);
}

TEST(Metrics, CrewUtilizationOfASingleTeam) {
  EXPECT_EQ(crewUtilizationPercent(planOf({{"L1", 0, 50, 1}, {"L2", 0, 30, 2}}), kJobs, {{"loader", 2}}, 100), 40.0);
  EXPECT_EQ(crewUtilizationPercent(planOf({}), kJobs, {}, 100), 0.0);
}

TEST(Metrics, CrewUtilizationIsPooledOverEveryCrewOnShift) {
  // 1 sorter busy the whole 100 minutes, 3 loaders idle: 100 of 400 paid crew-minutes.
  EXPECT_EQ(crewUtilizationPercent(planOf({{"S1", 0, 100, 1}}), kJobs, {{"loader", 3}, {"sorter", 1}}, 100), 25.0);
  // 2 loaders working 80 minutes, 2 sorters working 100: 180 of 400.
  EXPECT_EQ(crewUtilizationPercent(planOf({{"L1", 0, 50, 1}, {"L2", 0, 30, 2}, {"S1", 0, 100, 1}}), kJobs,
                                   {{"loader", 2}, {"sorter", 2}}, 100),
            45.0);
}

TEST(Metrics, TrucksOnTimeAndLastFinish) {
  std::vector<TruckStatus> board(3);
  board[0].state = TruckState::OnTime;
  board[1].state = TruckState::Late;
  board[2].state = TruckState::OnTime;
  EXPECT_EQ(trucksOnTime(board), 2);
  EXPECT_EQ(lastFinish(planOf({{"L1", 0, 50, 1}, {"S1", 20, 120, 1}})), 120);
  EXPECT_EQ(lastFinish(planOf({})), 0);
}
