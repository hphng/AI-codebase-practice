// Part A, level 4: waitlist, at station scale (rules 14-16 in src/locker/LockerBank.hpp).

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "locker/LockerBank.hpp"
#include "test_framework.hpp"

using namespace lockerhub;
using Ids = std::vector<std::string>;

namespace {
constexpr int kHold = 60;
}

TEST(LockerBankLevel4, ParcelThatDoesNotFitJoinsTheWaitlist) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  bank.deposit(0, "P1", Size::Small);
  const DepositResult r = bank.deposit(1, "P2", Size::Small);
  EXPECT_EQ(r.status, DepositStatus::NoSpace);
  EXPECT_EQ(r.compartment, "");
  bank.deposit(2, "P3", Size::Large);
  EXPECT_EQ(bank.waiting(), (Ids{"P2", "P3"}));

  EXPECT_FALSE(bank.pickup(3, "P2"));                                   // not in a locker yet
  EXPECT_EQ(bank.deposit(4, "P2", Size::Small).status, DepositStatus::Duplicate);  // already here
  EXPECT_EQ(bank.waiting(), (Ids{"P2", "P3"}));
}

TEST(LockerBankLevel4, EmptiedDoorGoesToTheWaitlistAtOnce) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  bank.deposit(0, "P1", Size::Small);
  bank.deposit(5, "P2", Size::Small);
  EXPECT_TRUE(bank.pickup(30, "P1"));
  EXPECT_EQ(bank.locate("P2").value_or("none"), "A01");
  EXPECT_EQ(bank.waiting(), Ids{});
  EXPECT_EQ(bank.freeCount(Size::Small), 0);

  // P2's window starts when it got the door (30), not when it arrived (5).
  bank.advance(90);
  EXPECT_EQ(bank.returned(), Ids{});
  bank.advance(91);
  EXPECT_EQ(bank.returned(), Ids{"P2"});
}

TEST(LockerBankLevel4, EarliestParcelThatFitsWinsNotTheFrontOfTheQueue) {
  LockerBank bank({{"A01", Size::Medium}, {"B01", Size::Large}}, kHold);
  bank.deposit(0, "P1", Size::Medium);  // A01
  bank.deposit(0, "P2", Size::Large);   // B01
  bank.deposit(1, "W1", Size::Large);
  bank.deposit(2, "W2", Size::Small);
  bank.deposit(3, "W3", Size::Medium);
  EXPECT_EQ(bank.waiting(), (Ids{"W1", "W2", "W3"}));

  // A medium door opens: W1 doesn't fit it; W2 joined before W3, so W2 gets it.
  bank.pickup(10, "P1");
  EXPECT_EQ(bank.locate("W2").value_or("none"), "A01");
  EXPECT_EQ(bank.waiting(), (Ids{"W1", "W3"}));

  // A large door opens: W1 fits and is the earliest.
  bank.pickup(11, "P2");
  EXPECT_EQ(bank.locate("W1").value_or("none"), "B01");
  EXPECT_EQ(bank.waiting(), Ids{"W3"});
}

TEST(LockerBankLevel4, DoorStaysEmptyIfNoWaitingParcelFits) {
  LockerBank bank({{"A01", Size::Small}, {"B01", Size::Large}}, kHold);
  bank.deposit(0, "P1", Size::Small);
  bank.deposit(0, "P2", Size::Large);
  bank.deposit(1, "W1", Size::Large);
  bank.pickup(10, "P1");
  EXPECT_EQ(bank.freeCount(Size::Small), 1);
  EXPECT_EQ(bank.waiting(), Ids{"W1"});
  // A new small parcel can still use the empty small door.
  EXPECT_EQ(bank.deposit(11, "P3", Size::Small).compartment, "A01");
}

TEST(LockerBankLevel4, ExpiriesHandTheirDoorsToTheWaitlistInExpiryOrder) {
  LockerBank bank({{"A01", Size::Small}, {"A02", Size::Small}}, kHold);
  bank.deposit(0, "X", Size::Small);  // A01
  bank.deposit(0, "Y", Size::Small);  // A02, window ends at 60
  bank.pickup(1, "X");
  bank.deposit(2, "Z", Size::Small);  // A01, window ends at 62
  bank.deposit(3, "W1", Size::Small);
  bank.deposit(4, "W2", Size::Small);

  // Y expires first, so its door (A02) goes to the front of the queue; then Z's door (A01) to W2.
  bank.advance(100);
  EXPECT_EQ(bank.returned(), (Ids{"Y", "Z"}));
  EXPECT_EQ(bank.locate("W1").value_or("none"), "A02");
  EXPECT_EQ(bank.locate("W2").value_or("none"), "A01");
  EXPECT_EQ(bank.longestWaiting(5), (Ids{"W1", "W2"}));
}

TEST(LockerBankLevel4, WaitlistedParcelsAreNeverReturned) {
  LockerBank bank({{"A01", Size::Small}}, kHold);
  bank.deposit(0, "P1", Size::Small);
  bank.deposit(0, "B1", Size::Large);  // no large doors at this station: waits
  bank.advance(1000);
  EXPECT_EQ(bank.returned(), Ids{"P1"});
  EXPECT_EQ(bank.waiting(), Ids{"B1"});
}

// A mega-station: 30,000 doors and ~100,000 calls. The morning rush overfills the station (30,000
// parcels on the waitlist, large ones queued first), the afternoon is pickups, the night is expiries.
TEST(LockerBankLevel4, RunsAMegaStationDayQuickly) {
  const int doors = 30000;
  std::vector<Compartment> compartments;
  for (int i = 0; i < doors; ++i) {
    const std::string n = std::to_string(100000 + i).substr(1);
    compartments.push_back({"K" + n, static_cast<Size>(i % 3)});
  }
  auto parcel = [](int i) { return "Q" + std::to_string(1000000 + i).substr(1); };

  const auto start = std::chrono::steady_clock::now();
  LockerBank bank(compartments, 5000);

  int time = 0;
  for (int i = 0; i < doors; ++i) bank.deposit(time + i / 100, parcel(i), static_cast<Size>(i % 3));
  time = 400;
  for (int i = 0; i < 30000; ++i) {
    const Size size = i < 15000 ? Size::Large : Size::Small;
    bank.deposit(time + i / 100, parcel(doors + i), size);
  }
  time = 800;
  std::uint32_t seed = 12345;
  for (int i = 0; i < 40000; ++i) {
    seed = seed * 1103515245u + 12345u;
    bank.pickup(time + i / 20, parcel(static_cast<int>((seed >> 8) % 60000)));
  }
  const std::size_t waitingAfterPickups = bank.waiting().size();
  const Ids oldestAfterPickups = bank.longestWaiting(2);
  for (int t = 3000; t <= 12000; t += 3) bank.advance(t);

  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
  std::cout << "    mega-station day took " << ms << " ms\n";

  EXPECT_EQ(waitingAfterPickups, 9942u);
  EXPECT_EQ(oldestAfterPickups, (Ids{"Q000000", "Q000001"}));
  ASSERT_EQ(bank.returned().size(), 39300u);
  EXPECT_EQ(bank.waiting().size(), 0u);
  EXPECT_EQ(bank.freeCount(Size::Small), 10000);
  EXPECT_EQ(bank.freeCount(Size::Large), 9358);
  EXPECT_EQ(bank.returned().front(), "Q000000");
  EXPECT_EQ(bank.returned().back(), "Q044357");
  EXPECT_TRUE(ms < 1500);
}
