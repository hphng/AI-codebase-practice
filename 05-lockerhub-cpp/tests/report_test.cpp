#include <fstream>
#include <sstream>
#include <string>

#include "report/Report.hpp"
#include "Records.hpp"
#include "test_framework.hpp"

using namespace lockerhub;

namespace {

std::string withoutCarriageReturns(std::string text) {
  std::string out;
  for (char c : text) {
    if (c != '\r') out += c;
  }
  return out;
}

}  // namespace

TEST(Report, RendersTheSummary) {
  DayLog log;
  log.activity = {"D1 08:00  CLOSE"};
  log.records = {testrecords::pickedUp("P1", 0, 90), testrecords::pickedUp("P2", 0, 30)};
  log.closedAt = 480;
  log.freeAtClose = {2, 1, 0};

  const std::string report = renderReport(StationConfig{"HUB-X", "X Hub", 48}, 3, log);
  EXPECT_CONTAINS(report, "Station: X Hub (HUB-X), 3 compartments, hold 48h");
  EXPECT_CONTAINS(report, "== Lockers at close (D1 08:00) ==");
  EXPECT_CONTAINS(report, "Empty compartments: S 2, M 1, L 0");
  EXPECT_CONTAINS(report, "Waitlist: none");
  EXPECT_CONTAINS(report, "Picked up:          2");
  EXPECT_CONTAINS(report, "Pickup rate:        100.0%");
  EXPECT_CONTAINS(report, "Average dwell:      1h 00m");
  EXPECT_CONTAINS(report, "Partner payout:     $0.80");
  EXPECT_CONTAINS(report, "Reminders due:      none");
}

TEST(Report, SampleDayMatchesTheExpectedReport) {
  std::ifstream file(std::string(SAMPLE_DATA_DIR) + "/expected_report.txt", std::ios::binary);
  ASSERT_TRUE(file.good());
  std::ostringstream expected;
  expected << file.rdbuf();
  EXPECT_EQ(runReport(SAMPLE_DATA_DIR), withoutCarriageReturns(expected.str()));
}
