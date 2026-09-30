// Part A, level 2: timing with unlimited crews (rules 5-7 in src/plan/Scheduler.hpp).

#include <chrono>
#include <iostream>

#include "JobBuilders.hpp"
#include "plan/Scheduler.hpp"
#include "test_framework.hpp"

using namespace shiftplan;
using namespace testjobs;

namespace {

std::string describe(const std::vector<Slot>& slots) {
  std::string out;
  for (const Slot& s : slots) out += s.job + "@" + std::to_string(s.start) + "-" + std::to_string(s.finish) + " ";
  return out;
}

}  // namespace

TEST(SchedulerLevel2, JobsStartWhenTheirLastPrerequisiteFinishes) {
  // U1 0-30, U2 0-50; S1 needs both, so it starts at 50 (not 30, and not 30+50).
  const std::vector<Job> jobs = {job("U1", 30, 5), job("U2", 50, 1), job("S1", 20, 9, {"U1", "U2"})};
  EXPECT_EQ(describe(earliestTimes(jobs)), "U1@0-30 U2@0-50 S1@50-70 ");
}

TEST(SchedulerLevel2, SlotsComeInDispatchOrderWithoutCrews) {
  const std::vector<Job> jobs = {job("A", 10, 1), job("B", 10, 9, {"A"}), job("C", 99, 5)};
  const auto slots = earliestTimes(jobs);
  EXPECT_EQ(describe(slots), "C@0-99 A@0-10 B@10-20 ");
  for (const Slot& s : slots) EXPECT_EQ(s.crew, 0);
}

TEST(SchedulerLevel2, CriticalPathFollowsTheLatestFinishingPrerequisite) {
  // The long single job X (0-100) doesn't beat the chain A -> B -> D (0-40, 40-70, 70-110).
  // D's other prerequisite C finishes at 50; B (70) is the one holding D back.
  const std::vector<Job> jobs = {
      job("X", 100, 9), job("A", 40, 1), job("B", 30, 1, {"A"}), job("C", 50, 7), job("D", 40, 1, {"C", "B"}),
  };
  EXPECT_EQ(criticalPath(jobs), (Ids{"A", "B", "D"}));
}

TEST(SchedulerLevel2, CriticalPathTiesGoToTheLowestId) {
  // Q and P both finish at 60 (tie at the end), and both of P's prerequisites finish at 30.
  const std::vector<Job> jobs = {
      job("N", 30, 1), job("M", 30, 1), job("Q", 60, 1), job("P", 30, 1, {"N", "M"}),
  };
  EXPECT_EQ(criticalPath(jobs), (Ids{"M", "P"}));
}

TEST(SchedulerLevel2, CycleAndEmptyInput) {
  const std::vector<Job> cycle = {job("A", 10, 1, {"B"}), job("B", 10, 1, {"A"})};
  EXPECT_THROW(earliestTimes(cycle), PlanError);
  EXPECT_THROW(criticalPath(cycle), PlanError);
  EXPECT_THROW(criticalPath({job("A", 10, 1, {"Z"})}), PlanError);
  EXPECT_EQ(earliestTimes({}).size(), 0u);
  EXPECT_EQ(criticalPath({}), Ids{});
  EXPECT_EQ(criticalPath({job("SOLO", 5, 1)}), Ids{"SOLO"});
}

// A big site: 100,000 jobs, ~150,000 prerequisite links.
TEST(SchedulerLevel2, PlansABigSiteQuickly) {
  const std::vector<Job> jobs = bigSite(100000);

  const auto start = std::chrono::steady_clock::now();
  const auto order = dispatchOrder(jobs);
  const auto slots = earliestTimes(jobs);
  const auto path = criticalPath(jobs);
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
  std::cout << "    big site (level 2) took " << ms << " ms\n";

  ASSERT_EQ(order.size(), 100000u);
  ASSERT_EQ(slots.size(), 100000u);
  ASSERT_TRUE(!path.empty());
  EXPECT_EQ(order.front(), "J000075");
  EXPECT_EQ(order[50000], "J007293");
  EXPECT_EQ(order.back(), "J099984");
  EXPECT_EQ(slots.back().finish, 51);
  EXPECT_EQ(path.size(), 613u);
  EXPECT_EQ(path.front(), "J000085");
  EXPECT_EQ(path.back(), "J099875");
  EXPECT_TRUE(ms < 3000);
}
