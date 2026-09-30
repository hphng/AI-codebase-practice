// Part A, level 3: pickup window (rules 9-13 in src/locker/LockerBank.hpp).

#include <stdexcept>
#include <string>
#include <vector>

#include "locker/LockerBank.hpp"
#include "test_framework.hpp"

using namespace lockerhub;
using Ids = std::vector<std::string>;

namespace {
constexpr int kHold = 60;
}

TEST(LockerBankLevel3, PickupOnTheLastMinuteOfTheWindowWorks) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  bank.deposit(100, "P1", Size::Small);
  EXPECT_TRUE(bank.pickup(160, "P1"));  // 100 + 60: still inside the window
  EXPECT_EQ(bank.returned(), Ids{});
}

TEST(LockerBankLevel3, ParcelIsReturnedOnceItsWindowEnds) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  bank.deposit(100, "P1", Size::Small);
  EXPECT_FALSE(bank.pickup(161, "P1"));
  EXPECT_EQ(bank.returned(), Ids{"P1"});
  EXPECT_FALSE(bank.locate("P1").has_value());
  EXPECT_EQ(bank.freeCount(Size::Small), 1);
}

TEST(LockerBankLevel3, DepositCanUseADoorEmptiedByAnExpiryInTheSameCall) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  bank.deposit(0, "P1", Size::Small);
  const DepositResult r = bank.deposit(61, "P2", Size::Small);
  EXPECT_EQ(r.status, DepositStatus::Stored);
  EXPECT_EQ(r.compartment, "A01");
  EXPECT_EQ(bank.returned(), Ids{"P1"});
}

TEST(LockerBankLevel3, ReturnsAreOrderedByEndOfWindowThenId) {
  LockerBank bank({{"A01", Size::Small}, {"A02", Size::Small}, {"A03", Size::Small}}, kHold);
  bank.deposit(0, "P2", Size::Small);  // window ends at 60
  bank.deposit(0, "P1", Size::Small);  // ends at 60
  bank.deposit(5, "P0", Size::Small);  // ends at 65
  bank.advance(100);
  EXPECT_EQ(bank.returned(), (Ids{"P1", "P2", "P0"}));
  EXPECT_EQ(bank.freeCount(Size::Small), 3);
}

TEST(LockerBankLevel3, AdvanceOnlyExpiresWhatHasEnded) {
  LockerBank bank({{"A01", Size::Small}, {"A02", Size::Small}}, kHold);
  bank.deposit(0, "P1", Size::Small);
  bank.deposit(30, "P2", Size::Small);
  bank.advance(60);
  EXPECT_EQ(bank.returned(), Ids{});
  bank.advance(61);
  EXPECT_EQ(bank.returned(), Ids{"P1"});
  EXPECT_EQ(bank.longestWaiting(5), Ids{"P2"});
  bank.advance(91);
  EXPECT_EQ(bank.returned(), (Ids{"P1", "P2"}));
}

TEST(LockerBankLevel3, PickedUpParcelsAreNeverReturned) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  bank.deposit(0, "P1", Size::Small);
  bank.pickup(10, "P1");
  bank.deposit(20, "P1", Size::Small);  // same id again: its new window ends at 80
  bank.advance(70);
  EXPECT_EQ(bank.returned(), Ids{});
  EXPECT_EQ(bank.locate("P1").value_or("none"), "A01");
  bank.advance(81);
  EXPECT_EQ(bank.returned(), Ids{"P1"});
}

TEST(LockerBankLevel3, TimeCannotGoBackwards) {
  LockerBank bank({{"A01", Size::Small}, {"A02", Size::Small}}, kHold);
  bank.deposit(50, "P1", Size::Small);
  EXPECT_THROW(bank.deposit(40, "P2", Size::Small), std::invalid_argument);
  EXPECT_FALSE(bank.locate("P2").has_value());
  EXPECT_THROW(bank.pickup(49, "P1"), std::invalid_argument);
  EXPECT_EQ(bank.locate("P1").value_or("none"), "A01");
  EXPECT_THROW(bank.advance(0), std::invalid_argument);

  EXPECT_EQ(bank.deposit(50, "P3", Size::Small).compartment, "A02");  // same minute is fine
}
