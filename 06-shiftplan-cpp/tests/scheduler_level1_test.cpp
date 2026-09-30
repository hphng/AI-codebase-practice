// Part A, level 1: dispatch order (rules 1-4 in src/plan/Scheduler.hpp).

#include "JobBuilders.hpp"
#include "plan/Scheduler.hpp"
#include "test_framework.hpp"

using namespace shiftplan;
using namespace testjobs;

TEST(SchedulerLevel1, JobsComeAfterTheirPrerequisites) {
  const std::vector<Job> jobs = {job("L1", 10, 9, {"S1"}), job("S1", 10, 9, {"U1"}), job("U1", 10, 1)};
  EXPECT_EQ(dispatchOrder(jobs), (Ids{"U1", "S1", "L1"}));
}

TEST(SchedulerLevel1, HighestPriorityOfTheJobsThatCanGoNext) {
  // B has the highest priority but needs A; of A and C, C has the higher priority.
  const std::vector<Job> jobs = {job("A", 10, 1), job("B", 10, 9, {"A"}), job("C", 10, 5)};
  EXPECT_EQ(dispatchOrder(jobs), (Ids{"C", "A", "B"}));
}

TEST(SchedulerLevel1, ReleasedJobsCompeteWithWaitingOnes) {
  // Finishing A releases D (priority 8), which then beats B and C that were available all along.
  const std::vector<Job> jobs = {job("A", 5, 9), job("B", 5, 4), job("C", 5, 3), job("D", 5, 8, {"A"})};
  EXPECT_EQ(dispatchOrder(jobs), (Ids{"A", "D", "B", "C"}));
}

TEST(SchedulerLevel1, TiesGoToTheLowestId) {
  const std::vector<Job> jobs = {job("S10", 5, 3), job("S2", 5, 3), job("S1", 5, 3)};
  EXPECT_EQ(dispatchOrder(jobs), (Ids{"S1", "S10", "S2"}));
}

TEST(SchedulerLevel1, ListingAPrerequisiteTwiceCountsOnce) {
  const std::vector<Job> jobs = {job("L1", 10, 1, {"S1", "U1", "S1"}), job("S1", 10, 1, {"U1", "U1"}), job("U1", 10, 1)};
  EXPECT_EQ(dispatchOrder(jobs), (Ids{"U1", "S1", "L1"}));
}

TEST(SchedulerLevel1, RejectsJobsThatCannotBePlanned) {
  EXPECT_THROW(dispatchOrder({job("", 10, 1)}), PlanError);
  EXPECT_THROW(dispatchOrder({job("A", 10, 1), job("A", 5, 1)}), PlanError);
  EXPECT_THROW(dispatchOrder({job("A", 0, 1)}), PlanError);
  EXPECT_THROW(dispatchOrder({job("A", 10, 1, {"NOPE"})}), PlanError);
  EXPECT_THROW(dispatchOrder({job("A", 10, 1, {"A"})}), PlanError);
  EXPECT_THROW(blockedJobs({job("A", -5, 1)}), PlanError);
}

TEST(SchedulerLevel1, ACycleHasNoOrder) {
  const std::vector<Job> jobs = {job("A", 10, 1, {"C"}), job("B", 10, 1, {"A"}), job("C", 10, 1, {"B"}), job("D", 10, 1)};
  EXPECT_THROW(dispatchOrder(jobs), PlanError);
}

TEST(SchedulerLevel1, BlockedJobsAreTheCycleAndEverythingBehindIt) {
  const std::vector<Job> jobs = {
      job("E", 10, 1),         job("B", 10, 1, {"A"}), job("A", 10, 1, {"B", "E"}),
      job("C", 10, 1, {"B"}),  job("D", 10, 1, {"C"}), job("F", 10, 1, {"E"}),
  };
  EXPECT_EQ(blockedJobs(jobs), (Ids{"A", "B", "C", "D"}));
  EXPECT_EQ(blockedJobs({job("A", 10, 1), job("B", 10, 1, {"A"})}), Ids{});
}

TEST(SchedulerLevel1, NoJobsNoOrder) {
  EXPECT_EQ(dispatchOrder({}), Ids{});
  EXPECT_EQ(blockedJobs({}), Ids{});
}
