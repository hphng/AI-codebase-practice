// StationDay replays events through the real LockerBank, so these tests need Part A.

#include <stdexcept>
#include <string>
#include <vector>

#include "station/StationDay.hpp"
#include "test_framework.hpp"

using namespace lockerhub;
using Ids = std::vector<std::string>;

namespace {

Event deposit(int minute, const std::string& id, Size size) {
  return Event{minute, EventKind::Deposit, id, size};
}
Event pickup(int minute, const std::string& id) {
  return Event{minute, EventKind::Pickup, id, Size::Small};
}
Event close(int minute) {
  return Event{minute, EventKind::Close, "", Size::Small};
}

const ParcelRecord& find(const DayLog& log, const std::string& id) {
  for (const auto& r : log.records) {
    if (r.id == id) return r;
  }
  throw std::runtime_error("no record for " + id);
}

}  // namespace

TEST(StationDay, RecordsEachParcelsDay) {
  const StationConfig station{"HUB-TEST", "Test Hub", 1};  // one-hour window
  const std::vector<Compartment> doors = {{"A01", Size::Small}, {"B01", Size::Medium}};
  const std::vector<Event> events = {
      deposit(0, "P1", Size::Small),  deposit(5, "P2", Size::Medium), deposit(6, "P3", Size::Small),
      deposit(7, "P1", Size::Small),  pickup(30, "P1"),                pickup(200, "P2"),
      close(200),
  };
  const DayLog log = runDay(station, doors, events);

  ASSERT_EQ(log.records.size(), 3u);
  EXPECT_TRUE(find(log, "P1").state == ParcelState::PickedUp);
  EXPECT_EQ(find(log, "P1").endedAt.value_or(-1), 30);
  // P2's window ended at 65, so it went back before the late pickup attempt.
  EXPECT_TRUE(find(log, "P2").state == ParcelState::Returned);
  EXPECT_EQ(find(log, "P2").endedAt.value_or(-1), 200);
  // P3 waited for a door and got A01 when P1 was picked up; its own window ended at 90.
  EXPECT_TRUE(find(log, "P3").waitlisted);
  EXPECT_EQ(find(log, "P3").placedAt.value_or(-1), 30);
  EXPECT_TRUE(find(log, "P3").state == ParcelState::Returned);

  EXPECT_EQ(log.closedAt, 200);
  EXPECT_EQ(log.freeAtClose[0], 1);
  EXPECT_EQ(log.freeAtClose[1], 1);
  EXPECT_EQ(log.waitlistAtClose, Ids{});
}

TEST(StationDay, LogsWhatHappenedInOrder) {
  const StationConfig station{"HUB-TEST", "Test Hub", 1};
  const std::vector<Compartment> doors = {{"A01", Size::Small}};
  const std::vector<Event> events = {
      deposit(480, "P1", Size::Small), deposit(481, "P2", Size::Small), pickup(482, "P1"), close(530),
  };
  const DayLog log = runDay(station, doors, events);

  const Ids expected = {
      "D1 08:00  DEPOSIT P1 (S)    -> A01",
      "D1 08:01  DEPOSIT P2 (S)    -> no space, added to the waitlist",
      "D1 08:02  PICKUP  P1        -> collected from A01",
      "D1 08:02  MOVE IN P2        -> A01 (from the waitlist)",
      "D1 08:50  CLOSE",
  };
  EXPECT_EQ(log.activity, expected);
  EXPECT_EQ(log.longestInLockers, Ids{"P2"});
}
