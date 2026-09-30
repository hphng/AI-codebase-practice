#include <string>
#include <vector>

#include "notify/Reminders.hpp"
#include "Records.hpp"
#include "test_framework.hpp"

using namespace lockerhub;
using namespace testrecords;
using Ids = std::vector<std::string>;

namespace {
constexpr int kDay = 24 * 60;
}

TEST(Reminders, ParcelInALockerForMoreThanADayGetsOne) {
  EXPECT_EQ(dueReminders({inLocker("P1", 0)}, kDay + 90), Ids{"P1"});
  EXPECT_EQ(dueReminders({inLocker("P1", 0)}, kDay - 1), Ids{});
}

TEST(Reminders, ParcelInALockerForExactlyADayGetsOne) {
  EXPECT_EQ(dueReminders({inLocker("P1", 600)}, 600 + kDay), Ids{"P1"});
}

TEST(Reminders, OnlyParcelsStillInALockerGetOne) {
  const std::vector<ParcelRecord> records = {
      pickedUp("P1", 0, 100),
      returnedToSender("P2", 0, 4000),
      waiting("P3"),
      inLocker("P4", 10),
  };
  EXPECT_EQ(dueReminders(records, 5000), Ids{"P4"});
}

TEST(Reminders, OldestFirstThenById) {
  const std::vector<ParcelRecord> records = {
      inLocker("P7", 300),
      inLocker("P9", 100),
      inLocker("P2", 300),
      inLocker("P1", 4000),  // not due yet
  };
  EXPECT_EQ(dueReminders(records, 300 + kDay + 60), (Ids{"P9", "P2", "P7"}));
}
