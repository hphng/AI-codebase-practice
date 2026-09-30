#include <stdexcept>
#include <string>
#include <vector>

#include "io/Clock.hpp"
#include "io/Csv.hpp"
#include "io/Loaders.hpp"
#include "model/Shift.hpp"
#include "test_framework.hpp"

using namespace shiftplan;
using Ids = std::vector<std::string>;

TEST(Clock, ParsesAndFormats) {
  EXPECT_EQ(parseClock("22:00"), 1320);
  EXPECT_EQ(parseClock("00:05"), 5);
  EXPECT_EQ(formatClock(1500), "01:00");
  EXPECT_EQ(formatClock(75), "01:15");
  EXPECT_THROW(parseClock("24:00"), std::invalid_argument);
  EXPECT_THROW(parseClock("7:30"), std::invalid_argument);
}

TEST(Shift, ConvertsBetweenClockAndShiftTime) {
  const Shift night{"n", 22 * 60, 480};
  EXPECT_EQ(night.minutesIntoShift(23 * 60 + 30), 90);
  EXPECT_EQ(night.minutesIntoShift(30), 150);
  EXPECT_EQ(night.minutesIntoShift(5 * 60), 420);
  EXPECT_EQ(night.clockAt(150), "00:30");
  EXPECT_EQ(night.clockAt(480), "06:00");
  const Shift day{"d", 6 * 60, 480};
  EXPECT_EQ(day.minutesIntoShift(9 * 60), 180);
}

TEST(Csv, SplitsTrimsAndSkipsComments) {
  const auto rows = parseCsv("a, b ,c\n\n# note\nd,,f\r\n");
  ASSERT_EQ(rows.size(), 2u);
  EXPECT_EQ(rows[0][1], "b");
  EXPECT_EQ(rows[1][1], "");
}

TEST(Loaders, ReadsJobsWithPrerequisiteLists) {
  const auto jobs = jobsFromRows({{"U1", "60", "10", "unloader", ""}, {"L4", "50", "35", "loader", "S4; S5;S4"}}, "jobs");
  ASSERT_EQ(jobs.size(), 2u);
  EXPECT_EQ(jobs[0].after, Ids{});
  EXPECT_EQ(jobs[1].after, (Ids{"S4", "S5", "S4"}));
  EXPECT_EQ(jobs[1].minutes, 50);
  EXPECT_EQ(jobs[1].crew, "loader");
}

TEST(Loaders, RejectsBadRows) {
  EXPECT_THROW(jobsFromRows({{"U1", "sixty", "10", "unloader", ""}}, "jobs"), std::runtime_error);
  EXPECT_THROW(crewsFromRows({{"sorter", "0"}}, "crews"), std::runtime_error);
  EXPECT_THROW(crewsFromRows({{"sorter", "2"}, {"sorter", "3"}}, "crews"), std::runtime_error);
  EXPECT_THROW(trucksFromRows({{"T1", "L1", "25:00"}}, "trucks"), std::runtime_error);
  EXPECT_THROW(shiftFromRows({{"site", "22:00", "0"}}, "shift"), std::runtime_error);
}

TEST(Loaders, ReadsTheSampleSite) {
  const SiteData site = loadSite(SAMPLE_DATA_DIR);
  EXPECT_EQ(site.shift.start, 22 * 60);
  EXPECT_EQ(site.shift.minutes, 480);
  EXPECT_EQ(site.crews.at("sorter"), 3);
  EXPECT_EQ(site.jobs.size(), 20u);
  EXPECT_EQ(site.trucks.size(), 5u);
}
