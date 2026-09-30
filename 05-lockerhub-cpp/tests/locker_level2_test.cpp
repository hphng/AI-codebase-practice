// Part A, level 2: queries (rules 6-8 in src/locker/LockerBank.hpp).

#include <string>
#include <vector>

#include "locker/LockerBank.hpp"
#include "test_framework.hpp"

using namespace lockerhub;
using Ids = std::vector<std::string>;

namespace {
constexpr int kHold = 600;

std::vector<Compartment> fourSmallDoors() {
  return {{"A01", Size::Small}, {"A02", Size::Small}, {"A03", Size::Small}, {"A04", Size::Small}};
}
}  // namespace

TEST(LockerBankLevel2, LocatesParcelsInCompartments) {
  LockerBank bank({{"A01", Size::Small}, {"B01", Size::Medium}}, kHold);
  bank.deposit(10, "P1", Size::Small);
  bank.deposit(11, "P2", Size::Medium);
  EXPECT_EQ(bank.locate("P1").value_or("none"), "A01");
  EXPECT_EQ(bank.locate("P2").value_or("none"), "B01");
  EXPECT_FALSE(bank.locate("P3").has_value());

  bank.pickup(20, "P1");
  EXPECT_FALSE(bank.locate("P1").has_value());
}

TEST(LockerBankLevel2, CountsEmptyCompartmentsPerSize) {
  LockerBank bank({{"A01", Size::Small}, {"A02", Size::Small}, {"B01", Size::Medium}, {"C01", Size::Large}}, kHold);
  EXPECT_EQ(bank.freeCount(Size::Small), 2);
  EXPECT_EQ(bank.freeCount(Size::Medium), 1);
  EXPECT_EQ(bank.freeCount(Size::Large), 1);

  bank.deposit(10, "P1", Size::Small);
  bank.deposit(11, "P2", Size::Small);
  bank.deposit(12, "P3", Size::Small);  // small doors are full, goes to the medium one
  EXPECT_EQ(bank.freeCount(Size::Small), 0);
  EXPECT_EQ(bank.freeCount(Size::Medium), 0);
  EXPECT_EQ(bank.freeCount(Size::Large), 1);

  bank.pickup(20, "P1");
  EXPECT_EQ(bank.freeCount(Size::Small), 1);
}

TEST(LockerBankLevel2, LongestWaitingIsEarliestPlacedThenById) {
  LockerBank bank(fourSmallDoors(), kHold);
  bank.deposit(5, "P5", Size::Small);
  bank.deposit(10, "P9", Size::Small);
  bank.deposit(10, "P3", Size::Small);
  bank.deposit(20, "P1", Size::Small);

  // P5 came first; P3 and P9 arrived in the same minute, so the id decides.
  EXPECT_EQ(bank.longestWaiting(3), (Ids{"P5", "P3", "P9"}));
  EXPECT_EQ(bank.longestWaiting(10), (Ids{"P5", "P3", "P9", "P1"}));
  EXPECT_EQ(bank.longestWaiting(0), Ids{});
  EXPECT_EQ(bank.longestWaiting(-2), Ids{});
}

TEST(LockerBankLevel2, LongestWaitingOnlyListsParcelsStillInALocker) {
  LockerBank bank(fourSmallDoors(), kHold);
  bank.deposit(5, "P5", Size::Small);
  bank.deposit(10, "P3", Size::Small);
  bank.deposit(20, "P1", Size::Small);
  bank.pickup(30, "P5");
  bank.deposit(40, "P5", Size::Small);  // same id, new stay: it is now the newest

  EXPECT_EQ(bank.longestWaiting(3), (Ids{"P3", "P1", "P5"}));
}
