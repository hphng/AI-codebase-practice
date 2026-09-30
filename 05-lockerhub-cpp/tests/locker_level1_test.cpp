// Part A, level 1: store and pick up (rules 1-5 in src/locker/LockerBank.hpp).

#include <stdexcept>

#include "locker/LockerBank.hpp"
#include "test_framework.hpp"

using namespace lockerhub;

namespace {
constexpr int kHold = 600;
}

TEST(LockerBankLevel1, StoresAParcelInAnEmptyCompartment) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  const DepositResult r = bank.deposit(10, "P1", Size::Small);
  EXPECT_EQ(r.status, DepositStatus::Stored);
  EXPECT_EQ(r.compartment, "A01");
}

TEST(LockerBankLevel1, UsesTheSmallestSizeClassThatFits) {
  // The large door has the lowest id, but a small parcel belongs in a small door first,
  // then a medium one, and only then a large one.
  LockerBank bank({{"A01", Size::Large}, {"B01", Size::Medium}, {"C01", Size::Small}}, kHold);
  EXPECT_EQ(bank.deposit(10, "P1", Size::Small).compartment, "C01");
  EXPECT_EQ(bank.deposit(11, "P2", Size::Small).compartment, "B01");
  EXPECT_EQ(bank.deposit(12, "P3", Size::Small).compartment, "A01");
}

TEST(LockerBankLevel1, UsesTheLowestIdWithinASizeClass) {
  LockerBank bank({{"C03", Size::Medium}, {"A12", Size::Medium}, {"B01", Size::Medium}, {"A02", Size::Large}},
                  kHold);
  EXPECT_EQ(bank.deposit(10, "P1", Size::Medium).compartment, "A12");
  EXPECT_EQ(bank.deposit(10, "P2", Size::Medium).compartment, "B01");
  EXPECT_EQ(bank.deposit(10, "P3", Size::Medium).compartment, "C03");
  EXPECT_EQ(bank.deposit(10, "P4", Size::Medium).compartment, "A02");
}

TEST(LockerBankLevel1, ReportsNoSpaceWhenNothingFits) {
  LockerBank bank({{"A01", Size::Small}, {"B01", Size::Medium}}, kHold);
  const DepositResult large = bank.deposit(10, "P1", Size::Large);
  EXPECT_EQ(large.status, DepositStatus::NoSpace);
  EXPECT_EQ(large.compartment, "");

  EXPECT_EQ(bank.deposit(11, "P2", Size::Medium).status, DepositStatus::Stored);
  EXPECT_EQ(bank.deposit(12, "P3", Size::Medium).status, DepositStatus::NoSpace);
  EXPECT_EQ(bank.deposit(13, "P4", Size::Small).compartment, "A01");
}

TEST(LockerBankLevel1, RefusesAParcelThatIsAlreadyInALocker) {
  LockerBank bank({{"A01", Size::Small}, {"A02", Size::Small}}, kHold);
  EXPECT_EQ(bank.deposit(10, "P1", Size::Small).compartment, "A01");
  const DepositResult again = bank.deposit(11, "P1", Size::Small);
  EXPECT_EQ(again.status, DepositStatus::Duplicate);
  EXPECT_EQ(again.compartment, "");
  // The refused deposit didn't take a door.
  EXPECT_EQ(bank.deposit(12, "P2", Size::Small).compartment, "A02");
}

TEST(LockerBankLevel1, PickupEmptiesTheCompartment) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  bank.deposit(10, "P1", Size::Small);
  EXPECT_TRUE(bank.pickup(20, "P1"));
  EXPECT_EQ(bank.deposit(30, "P2", Size::Small).compartment, "A01");
}

TEST(LockerBankLevel1, PickupOfAParcelThatIsNotThereFails) {
  LockerBank bank({{"A01", Size::Small}, {"A02", Size::Small}}, kHold);
  bank.deposit(10, "P1", Size::Small);
  EXPECT_FALSE(bank.pickup(20, "NOPE"));
  EXPECT_TRUE(bank.pickup(21, "P1"));
  EXPECT_FALSE(bank.pickup(22, "P1"));
  EXPECT_EQ(bank.deposit(23, "P2", Size::Small).compartment, "A01");
  EXPECT_EQ(bank.deposit(24, "P3", Size::Small).compartment, "A02");
}

TEST(LockerBankLevel1, ParcelIdCanComeBackAfterPickup) {
  LockerBank bank({{"A01", Size::Small}, {"B01", Size::Large}}, kHold);
  bank.deposit(10, "P1", Size::Large);
  EXPECT_TRUE(bank.pickup(20, "P1"));
  const DepositResult back = bank.deposit(30, "P1", Size::Small);
  EXPECT_EQ(back.status, DepositStatus::Stored);
  EXPECT_EQ(back.compartment, "A01");
}

TEST(LockerBankLevel1, ConstructorRejectsBadSetup) {
  EXPECT_THROW(LockerBank({{"A01", Size::Small}, {"A01", Size::Large}}, kHold), std::invalid_argument);
  EXPECT_THROW(LockerBank({{"A01", Size::Small}}, 0), std::invalid_argument);
  EXPECT_NO_THROW(LockerBank({}, kHold));
}
