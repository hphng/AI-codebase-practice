#include "report/Departures.hpp"
#include "test_framework.hpp"

using namespace shiftplan;

namespace {

const Shift kNight{"test", 22 * 60, 480};  // 22:00 - 06:00

ShiftPlan planWith(const std::string& job, int start, int finish) {
  ShiftPlan plan;
  plan.slots.push_back(Slot{job, start, finish, 1});
  return plan;
}

}  // namespace

TEST(Departures, SealedExactlyAtTheCutoffIsOnTime) {
  // 23:30 is 90 minutes into the shift; loading done at 75 + 15 minutes to seal = 90.
  const auto board = truckStatuses({{"T1", "L1", 23 * 60 + 30}}, planWith("L1", 40, 75), kNight);
  ASSERT_EQ(board.size(), 1u);
  EXPECT_TRUE(board[0].state == TruckState::OnTime);
  EXPECT_EQ(board[0].sealedAt.value_or(-1), 90);
  EXPECT_EQ(board[0].cutoff, 90);

  const auto late = truckStatuses({{"T1", "L1", 23 * 60 + 30}}, planWith("L1", 40, 76), kNight);
  EXPECT_TRUE(late[0].state == TruckState::Late);
}

TEST(Departures, TrucksLeavingAfterMidnightAreJudgedAgainstTheirOwnCutoff) {
  // 02:00 is 240 minutes into a shift that started at 22:00.
  const auto board = truckStatuses({{"T3", "L3", 2 * 60}}, planWith("L3", 100, 180), kNight);
  EXPECT_EQ(board[0].cutoff, 240);
  EXPECT_TRUE(board[0].state == TruckState::OnTime);

  const auto late = truckStatuses({{"T3", "L3", 2 * 60}}, planWith("L3", 100, 230), kNight);
  EXPECT_TRUE(late[0].state == TruckState::Late);
}

TEST(Departures, TruckWhoseLoadDidNotRunIsCarriedOver) {
  ShiftPlan plan = planWith("L1", 0, 30);
  plan.carriedOver = {"L2"};
  const auto board = truckStatuses({{"T1", "L1", 23 * 60}, {"T2", "L2", 23 * 60}}, plan, kNight);
  ASSERT_EQ(board.size(), 2u);
  EXPECT_TRUE(board[0].state == TruckState::OnTime);
  EXPECT_TRUE(board[1].state == TruckState::CarriedOver);
  EXPECT_FALSE(board[1].sealedAt.has_value());
  EXPECT_EQ(board[1].loadJob, "L2");
}

TEST(Departures, DayShiftCutoffs) {
  const Shift day{"test", 6 * 60, 480};  // 06:00 - 14:00
  const auto board = truckStatuses({{"T1", "L1", 9 * 60}}, planWith("L1", 100, 165), day);
  EXPECT_EQ(board[0].cutoff, 180);
  EXPECT_TRUE(board[0].state == TruckState::OnTime);
}
