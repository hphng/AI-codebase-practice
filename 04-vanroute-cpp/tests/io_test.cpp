#include <stdexcept>

#include "io/Csv.hpp"
#include "io/Loaders.hpp"
#include "test_framework.hpp"

using namespace vanroute;

TEST(Csv, SplitsAndTrimsFields) {
  const auto rows = parseCsv("a, b ,c\n 1,2 , 3 \n");
  ASSERT_EQ(rows.size(), static_cast<std::size_t>(2));
  EXPECT_EQ(rows[0].fields.size(), static_cast<std::size_t>(3));
  EXPECT_EQ(rows[0].fields[1], std::string("b"));
  EXPECT_EQ(rows[1].fields[2], std::string("3"));
  EXPECT_EQ(rows[1].line, 2);
}

TEST(Csv, SkipsBlankLinesAndHandlesCrlf) {
  const auto rows = parseCsv("x,1\r\n\r\n   \r\ny,2\r\n");
  ASSERT_EQ(rows.size(), static_cast<std::size_t>(2));
  EXPECT_EQ(rows[1].fields[1], std::string("2"));
  EXPECT_EQ(rows[1].line, 4);
}

TEST(Csv, StripsTrailingComments) {
  const auto rows = parseCsv("V1,DEPOT,6   # the big van\n");
  ASSERT_EQ(rows.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(rows[0].fields.size(), static_cast<std::size_t>(3));
  EXPECT_EQ(rows[0].fields[2], std::string("6"));
}

TEST(Csv, SkipsFullLineComments) {
  const auto rows = parseCsv("# id,destination,size\n#another note\nP1,N1,2\n  # indented comment\n");
  ASSERT_EQ(rows.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(rows[0].fields[0], std::string("P1"));
  EXPECT_EQ(rows[0].line, 3);
}

TEST(Csv, KeepsTrailingEmptyField) {
  const auto rows = parseCsv("a,b,\n");
  ASSERT_EQ(rows.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(rows[0].fields.size(), static_cast<std::size_t>(3));
}

TEST(Csv, IntegerFields) {
  const auto rows = parseCsv("12,x,7y\n");
  EXPECT_EQ(toInt(rows[0], 0), 12);
  EXPECT_THROW(toInt(rows[0], 1), std::runtime_error);
  EXPECT_THROW(toInt(rows[0], 2), std::runtime_error);
  EXPECT_THROW(toInt(rows[0], 9), std::runtime_error);
}

TEST(Clock, ParsesAndFormats) {
  EXPECT_EQ(parseClock("08:30"), 510);
  EXPECT_EQ(parseClock("00:00"), 0);
  EXPECT_EQ(parseClock("23:59"), 1439);
  EXPECT_THROW(parseClock("8:30"), std::runtime_error);
  EXPECT_THROW(parseClock("24:00"), std::runtime_error);
  EXPECT_EQ(formatClock(510), std::string("08:30"));
  EXPECT_EQ(formatClock(65), std::string("01:05"));
}

TEST(Loaders, Roads) {
  const RoadGraph g = loadRoads(parseCsv("A,B,5\nB,C,7\nA,B,3\n"));
  EXPECT_EQ(g.nodeCount(), static_cast<std::size_t>(3));
  EXPECT_EQ(g.neighbors(g.indexOf("B")).size(), static_cast<std::size_t>(3));
  EXPECT_EQ(g.indexOf("nope"), -1);
  EXPECT_THROW(loadRoads(parseCsv("A,B\n")), std::runtime_error);
  EXPECT_THROW(loadRoads(parseCsv("A,B,-1\n")), std::invalid_argument);
}

TEST(Loaders, VansAndParcels) {
  const auto vans = loadVans(parseCsv("V1,DEPOT,6,08:00,12:00\n"));
  ASSERT_EQ(vans.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(vans[0].capacity, 6);
  EXPECT_EQ(vans[0].shiftStart, 480);

  const auto parcels = loadParcels(parseCsv("P1,N4,2,09:15,ready\nP2,N5,1,10:00,cancelled\n"));
  ASSERT_EQ(parcels.size(), static_cast<std::size_t>(2));
  EXPECT_EQ(parcels[0].deadline, 555);
  EXPECT_FALSE(parcels[0].cancelled);
  EXPECT_TRUE(parcels[1].cancelled);
  EXPECT_THROW(loadParcels(parseCsv("P1,N4,2,09:15,lost\n")), std::runtime_error);
}
