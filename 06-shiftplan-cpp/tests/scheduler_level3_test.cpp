// Part A, level 3: a limited number of crews (rules 8-9 in src/plan/Scheduler.hpp).

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

TEST(SchedulerLevel3, OneCrewWorksThroughReadyJobsByPriority) {
  const std::vector<Job> jobs = {job("A", 10, 1), job("B", 20, 9, {"A"}), job("C", 5, 5)};
  EXPECT_EQ(describe(scheduleCrews(jobs, 1)), "C@0-5#1 A@5-15#1 B@15-35#1 ");
}

TEST(SchedulerLevel3, ACrewNeverWaitsWhileAJobIsReady) {
  // Dispatch order is A, B, C. With two crews, C must not wait until B is placed: crew 2 takes it at 0.
  const std::vector<Job> jobs = {job("A", 10, 2), job("B", 1, 9, {"A"}), job("C", 1, 0)};
  EXPECT_EQ(describe(scheduleCrews(jobs, 2)), "A@0-10#1 C@0-1#2 B@10-11#1 ");
}

TEST(SchedulerLevel3, AFinishedJobFreesItsCrewAndReleasesJobsAtTheSameMoment) {
  // At 30, A finishes: crew 1 is free and B (priority 9) is ready at that same moment, so B beats C.
  const std::vector<Job> jobs = {job("A", 30, 5), job("C", 10, 1), job("B", 10, 9, {"A"}), job("D", 30, 4)};
  EXPECT_EQ(describe(scheduleCrews(jobs, 2)), "A@0-30#1 D@0-30#2 B@30-40#1 C@30-40#2 ");
}

TEST(SchedulerLevel3, TheLowestNumberedFreeCrewTakesTheJob) {
  // Crew 1 frees up at 10, crew 3 at 5, crew 2 at 20. E arrives when both 1 and 3 are free.
  const std::vector<Job> jobs = {
      job("X", 10, 9), job("Y", 20, 8), job("Z", 5, 7), job("E", 4, 1, {"X"}),
  };
  EXPECT_EQ(describe(scheduleCrews(jobs, 3)), "X@0-10#1 Y@0-20#2 Z@0-5#3 E@10-14#1 ");
}

TEST(SchedulerLevel3, SlotsAreSortedByStartThenCrew) {
  const std::vector<Job> jobs = {
      job("A", 50, 1), job("B", 10, 9), job("C", 10, 8, {"B"}), job("D", 10, 7, {"C"}),
  };
  EXPECT_EQ(describe(scheduleCrews(jobs, 2)), "B@0-10#1 A@0-50#2 C@10-20#1 D@20-30#1 ");
}

TEST(SchedulerLevel3, AnyCrewCanDoAnyJobInThisLevel) {
  const std::vector<Job> jobs = {job("A", 10, 1, {}, "loader"), job("B", 10, 2, {}, "sorter")};
  EXPECT_EQ(describe(scheduleCrews(jobs, 1)), "B@0-10#1 A@10-20#1 ");
}

TEST(SchedulerLevel3, BadInput) {
  EXPECT_THROW(scheduleCrews({job("A", 10, 1)}, 0), std::invalid_argument);
  EXPECT_THROW(scheduleCrews({job("A", 10, 1, {"B"}), job("B", 10, 1, {"A"})}, 2), PlanError);
  EXPECT_EQ(scheduleCrews({}, 3).size(), 0u);
}
