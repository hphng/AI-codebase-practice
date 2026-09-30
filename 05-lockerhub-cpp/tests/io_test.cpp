#include <stdexcept>
#include <string>
#include <vector>

#include "io/Clock.hpp"
#include "io/Csv.hpp"
#include "io/Loaders.hpp"
#include "model/Size.hpp"
#include "test_framework.hpp"

using namespace lockerhub;

TEST(Size, ParsesCodesAndChecksFit) {
  EXPECT_TRUE(parseSize("M") == Size::Medium);
  EXPECT_EQ(sizeCode(Size::Large), 'L');
  EXPECT_THROW(parseSize("XL"), std::invalid_argument);
  EXPECT_TRUE(fits(Size::Small, Size::Large));
  EXPECT_TRUE(fits(Size::Medium, Size::Medium));
  EXPECT_FALSE(fits(Size::Large, Size::Medium));
}

TEST(Clock, ParsesAndFormatsStamps) {
  EXPECT_EQ(parseStamp("D1", "08:00"), 480);
  EXPECT_EQ(parseStamp("D2", "00:30"), 1470);
  EXPECT_EQ(formatStamp(1470), "D2 00:30");
  EXPECT_EQ(formatStamp(parseStamp("D12", "23:59")), "D12 23:59");
  EXPECT_THROW(parseStamp("D0", "08:00"), std::invalid_argument);
  EXPECT_THROW(parseStamp("D1", "24:00"), std::invalid_argument);
  EXPECT_THROW(parseStamp("1", "08:00"), std::invalid_argument);
}

TEST(Clock, FormatsDurations) {
  EXPECT_EQ(formatDuration(1265), "21h 05m");
  EXPECT_EQ(formatDuration(45), "0h 45m");
  EXPECT_EQ(formatDuration(3000), "50h 00m");
}

TEST(Csv, SplitsTrimsAndSkipsComments) {
  const auto rows = parseCsv("a, b ,c\n\n# note\nd,,f\r\n");
  ASSERT_EQ(rows.size(), 2u);
  EXPECT_EQ(rows[0][1], "b");
  EXPECT_EQ(rows[1][1], "");
  EXPECT_EQ(rows[1][2], "f");
}

TEST(Loaders, ReadsTheSampleStation) {
  const StationConfig station = loadStation(std::string(SAMPLE_DATA_DIR) + "/station.csv");
  EXPECT_EQ(station.id, "HUB-SEA-14");
  EXPECT_EQ(station.holdHours, 48);
  EXPECT_EQ(loadCompartments(std::string(SAMPLE_DATA_DIR) + "/lockers.csv").size(), 7u);
  EXPECT_EQ(loadEvents(std::string(SAMPLE_DATA_DIR) + "/events.csv").size(), 28u);
}

TEST(Loaders, ReadsEvents) {
  const auto events = eventsFromRows({{"D1", "08:00", "DEPOSIT", "P1", "M"},
                                      {"D1", "09:00", "PICKUP", "P1", ""},
                                      {"D2", "20:00", "CLOSE", "", ""}},
                                     "events.csv");
  ASSERT_EQ(events.size(), 3u);
  EXPECT_TRUE(events[0].kind == EventKind::Deposit);
  EXPECT_TRUE(events[0].size == Size::Medium);
  EXPECT_EQ(events[1].minute, 540);
  EXPECT_TRUE(events[2].kind == EventKind::Close);
}

TEST(Loaders, RejectsBadRows) {
  EXPECT_THROW(eventsFromRows({{"D1", "08:00", "DEPOSIT", "P1", "XL"}}, "e"), std::runtime_error);
  EXPECT_THROW(eventsFromRows({{"D1", "08:00", "RETURN", "P1", ""}}, "e"), std::runtime_error);
  EXPECT_THROW(eventsFromRows({{"D1", "08:00", "PICKUP", "P1", "S"}}, "e"), std::runtime_error);
  EXPECT_THROW(eventsFromRows({{"D1", "09:00", "PICKUP", "P1", ""}, {"D1", "08:00", "PICKUP", "P2", ""}}, "e"),
               std::runtime_error);
  EXPECT_THROW(compartmentsFromRows({{"A01", "S"}, {"A01", "M"}}, "l"), std::runtime_error);
  EXPECT_THROW(stationFromRows({{"HUB", "Name", "0"}}, "s"), std::runtime_error);
}
