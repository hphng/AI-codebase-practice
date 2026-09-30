// Part A, level 4: crew types and the end of the shift, at site scale (rules 10-13).

#include <chrono>
#include <iostream>
#include <stdexcept>

#include "JobBuilders.hpp"
#include "plan/Scheduler.hpp"
#include "test_framework.hpp"

using namespace shiftplan;
using namespace testjobs;

namespace {

std::string describe(const std::vector<Slot>& slots) {
  std::string out;
  for (const Slot& s : slots) {
    out += s.job + "@" + std::to_string(s.start) + "-" + std::to_string(s.finish) + "#" + std::to_string(s.crew) + " ";
  }
  return out;
}

}  // namespace

TEST(SchedulerLevel4, EachCrewOnlyTakesJobsOfItsOwnType) {
  // One unloader, one sorter. The sorter doesn't help with the second unload.
  const std::vector<Job> jobs = {
      job("U1", 30, 9, {}, "unloader"), job("U2", 30, 5, {}, "unloader"), job("S1", 20, 1, {"U1"}, "sorter"),
  };
  const ShiftPlan plan = planShift(jobs, {{"unloader", 1}, {"sorter", 1}}, 480);
  EXPECT_EQ(describe(plan.slots), "U1@0-30#1 S1@30-50#1 U2@30-60#1 ");
  EXPECT_EQ(plan.carriedOver, Ids{});
}

TEST(SchedulerLevel4, SlotsAreSortedByStartThenCrewTypeThenCrew) {
  const std::vector<Job> jobs = {
      job("U1", 10, 1, {}, "unloader"), job("L1", 10, 1, {}, "loader"), job("L2", 10, 2, {}, "loader"),
      job("S1", 10, 1, {}, "sorter"),
  };
  const ShiftPlan plan = planShift(jobs, {{"unloader", 1}, {"sorter", 1}, {"loader", 2}}, 480);
  EXPECT_EQ(describe(plan.slots), "L2@0-10#1 L1@0-10#2 S1@0-10#1 U1@0-10#1 ");
}

TEST(SchedulerLevel4, AJobMayFinishExactlyAtTheEndOfTheShift) {
  const ShiftPlan plan = planShift({job("A", 60, 1, {}, "x"), job("B", 40, 1, {"A"}, "x")}, {{"x", 1}}, 100);
  EXPECT_EQ(describe(plan.slots), "A@0-60#1 B@60-100#1 ");
  EXPECT_EQ(plan.carriedOver, Ids{});
}

TEST(SchedulerLevel4, WorkThatCannotFinishIsCarriedOverWithEverythingThatNeedsIt) {
  // BIG (priority 9) can't finish by 100, so the crew takes SMALL instead of waiting.
  // AFTER needs BIG and FINAL needs AFTER: both carried over too.
  const std::vector<Job> jobs = {
      job("BIG", 120, 9, {}, "x"), job("SMALL", 50, 1, {}, "x"), job("AFTER", 5, 5, {"BIG"}, "x"),
      job("FINAL", 5, 5, {"AFTER", "SMALL"}, "x"), job("LATE", 40, 3, {"SMALL"}, "x"),
  };
  const ShiftPlan plan = planShift(jobs, {{"x", 1}}, 100);
  EXPECT_EQ(describe(plan.slots), "SMALL@0-50#1 LATE@50-90#1 ");
  EXPECT_EQ(plan.carriedOver, (Ids{"AFTER", "BIG", "FINAL"}));
}

TEST(SchedulerLevel4, AJobThatNoLongerFitsDoesNotBlockShorterOnes) {
  // At 60 the crew is free: LONG (priority 9) needs 50 minutes but only 40 are left; SHORT fits.
  const std::vector<Job> jobs = {
      job("FIRST", 60, 9, {}, "x"), job("LONG", 50, 9, {"FIRST"}, "x"), job("SHORT", 30, 1, {"FIRST"}, "x"),
  };
  const ShiftPlan plan = planShift(jobs, {{"x", 1}}, 100);
  EXPECT_EQ(describe(plan.slots), "FIRST@0-60#1 SHORT@60-90#1 ");
  EXPECT_EQ(plan.carriedOver, Ids{"LONG"});
}

TEST(SchedulerLevel4, BadInput) {
  EXPECT_THROW(planShift({job("A", 10, 1, {}, "welder")}, {{"x", 1}}, 100), PlanError);
  EXPECT_THROW(planShift({job("A", 10, 1, {}, "x")}, {{"x", 0}}, 100), std::invalid_argument);
  EXPECT_THROW(planShift({job("A", 10, 1, {}, "x")}, {{"x", 1}}, 0), std::invalid_argument);
  EXPECT_THROW(planShift({job("A", 10, 1, {"B"}, "x"), job("B", 10, 1, {"A"}, "x")}, {{"x", 1}}, 100), PlanError);
}

// A big site's shift: 100,000 jobs, 4 crew types with 40 crews each, a shift that is too short for all
// of it.
TEST(SchedulerLevel4, PlansABigSiteShiftQuickly) {
  const std::vector<Job> jobs = bigSite(100000);
  const CrewPool crews = {{"a", 40}, {"b", 40}, {"c", 40}, {"d", 40}};

  const auto start = std::chrono::steady_clock::now();
  const ShiftPlan plan = planShift(jobs, crews, 24000);
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
  std::cout << "    big site shift (level 4) took " << ms << " ms\n";

  ASSERT_TRUE(!plan.slots.empty());
  EXPECT_EQ(plan.slots.size(), 81198u);
  EXPECT_EQ(plan.carriedOver.size(), 18802u);
  EXPECT_EQ(plan.slots.back().job, "J063861");
  EXPECT_EQ(plan.slots.back().finish, 23992);
  EXPECT_EQ(plan.slots[40000].job, "J031181");
  EXPECT_TRUE(ms < 3000);
}
