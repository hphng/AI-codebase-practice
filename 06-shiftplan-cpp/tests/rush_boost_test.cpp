#include "JobBuilders.hpp"
#include "plan/RushBoost.hpp"
#include "test_framework.hpp"

using namespace shiftplan;
using namespace testjobs;

namespace {

const Shift kNight{"test", 22 * 60, 480};  // 22:00 - 06:00

int priorityOf(const RushBoost& r, const std::string& id) {
  for (const Job& j : r.jobs) {
    if (j.id == id) return j.priority;
  }
  return -1;
}

}  // namespace

TEST(RushBoost, TrucksLeavingEarlyInTheShiftAreRushTrucks) {
  const std::vector<Job> jobs = {job("L1", 10, 1), job("L2", 10, 1), job("L3", 10, 1)};
  const std::vector<Truck> trucks = {{"T1", "L1", 23 * 60}, {"T2", "L2", 59}, {"T3", "L3", 60}};
  // 23:00 is 60 minutes in, 00:59 is 179 minutes in (rush); 01:00 is exactly 180 (not rush).
  const RushBoost r = applyRushBoost(jobs, trucks, kNight);
  EXPECT_EQ(r.rushTrucks, (Ids{"T1", "T2"}));
  EXPECT_EQ(r.boosted, (Ids{"L1", "L2"}));
  EXPECT_EQ(priorityOf(r, "L3"), 1);
}

TEST(RushBoost, EveryJobUpstreamOfARushTruckIsBoosted) {
  const std::vector<Job> jobs = {
      job("U1", 60, 10), job("S1", 40, 20, {"U1"}), job("L1", 30, 30, {"S1"}), job("U9", 60, 50),
  };
  const RushBoost r = applyRushBoost(jobs, {{"T1", "L1", 30}}, kNight);  // 00:30
  EXPECT_EQ(r.boosted, (Ids{"L1", "S1", "U1"}));
  EXPECT_EQ(priorityOf(r, "U1"), 110);
  EXPECT_EQ(priorityOf(r, "S1"), 120);
  EXPECT_EQ(priorityOf(r, "L1"), 130);
  EXPECT_EQ(priorityOf(r, "U9"), 50);
}

TEST(RushBoost, AJobFeedingTwoRushTrucksIsBoostedOnce) {
  const std::vector<Job> jobs = {job("S1", 40, 20), job("L1", 30, 30, {"S1"}), job("L2", 30, 30, {"S1"})};
  const RushBoost r = applyRushBoost(jobs, {{"T1", "L1", 30}, {"T2", "L2", 45}}, kNight);
  EXPECT_EQ(r.boosted, (Ids{"L1", "L2", "S1"}));
  EXPECT_EQ(priorityOf(r, "S1"), 120);
}

TEST(RushBoost, KeepsTheJobListInItsOriginalOrder) {
  const std::vector<Job> jobs = {job("B", 10, 1), job("A", 10, 2), job("C", 10, 3, {"A"})};
  const RushBoost r = applyRushBoost(jobs, {{"T9", "C", 5 * 60}}, kNight);  // 05:00: not a rush truck
  ASSERT_EQ(r.jobs.size(), 3u);
  EXPECT_EQ(r.jobs[0].id, "B");
  EXPECT_EQ(r.jobs[2].id, "C");
  EXPECT_EQ(r.jobs[2].after, Ids{"A"});
  EXPECT_EQ(r.boosted, Ids{});
  EXPECT_EQ(r.rushTrucks, Ids{});
}
