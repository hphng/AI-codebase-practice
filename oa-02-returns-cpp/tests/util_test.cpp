#include <stdexcept>

#include "test_framework.hpp"
#include "util/Date.hpp"
#include "util/Money.hpp"
#include "util/Strings.hpp"

using namespace returns;

// ---------- Date ----------

TEST(Date, ParsesIsoDates) {
  const Date d = Date::parse("2024-03-09");
  EXPECT_EQ(d.year, 2024);
  EXPECT_EQ(d.month, 3);
  EXPECT_EQ(d.day, 9);
  EXPECT_EQ(d.toString(), std::string("2024-03-09"));
}

TEST(Date, RejectsMalformedAndImpossibleDates) {
  EXPECT_THROW(Date::parse("2024-3-09"), std::invalid_argument);
  EXPECT_THROW(Date::parse("2024-13-01"), std::invalid_argument);
  EXPECT_THROW(Date::parse("2024-04-31"), std::invalid_argument);
  EXPECT_THROW(Date::parse("2023-02-29"), std::invalid_argument);
  EXPECT_THROW(Date::parse("1900-02-28"), std::invalid_argument);  // before 1970
}

TEST(Date, LeapYearRules) {
  EXPECT_TRUE(isLeapYear(2024));
  EXPECT_FALSE(isLeapYear(2023));
  EXPECT_FALSE(isLeapYear(2100));
  EXPECT_TRUE(isLeapYear(2000));
  EXPECT_TRUE(isLeapYear(2400));
}

TEST(Date, AcceptsFebruary29InLeapYears) {
  EXPECT_NO_THROW(Date::parse("2024-02-29"));
  EXPECT_NO_THROW(Date::parse("2000-02-29"));
  EXPECT_THROW(Date::parse("2100-02-29"), std::invalid_argument);
}

TEST(Date, DaysBetween) {
  EXPECT_EQ(daysBetween(Date::parse("2024-01-01"), Date::parse("2024-01-31")), 30);
  EXPECT_EQ(daysBetween(Date::parse("2023-12-31"), Date::parse("2024-01-01")), 1);
  EXPECT_EQ(daysBetween(Date::parse("2024-02-28"), Date::parse("2024-03-01")), 2);
  EXPECT_EQ(daysBetween(Date::parse("2024-03-10"), Date::parse("2024-03-01")), -9);
}

TEST(Date, DaysBetweenAcrossCenturyLeapYear) {
  EXPECT_EQ(daysBetween(Date::parse("2000-02-28"), Date::parse("2000-03-01")), 2);
  EXPECT_EQ(daysBetween(Date::parse("1999-01-01"), Date::parse("2001-01-01")), 731);
}

TEST(Date, EpochDayNumbers) {
  EXPECT_EQ(Date::parse("1970-01-01").toDays(), 0);
  EXPECT_EQ(Date::parse("1971-01-01").toDays(), 365);
}

// ---------- Money ----------

TEST(Money, PercentOfWholeResults) {
  EXPECT_EQ(percentOf(5000, 10), 500);
  EXPECT_EQ(percentOf(5000, 0), 0);
  EXPECT_EQ(percentOf(5000, 100), 5000);
  EXPECT_EQ(percentOf(0, 15), 0);
}

TEST(Money, PercentOfRoundsHalfUpToTheCent) {
  EXPECT_EQ(percentOf(1999, 15), 300);  // 299.85
  EXPECT_EQ(percentOf(1990, 15), 299);  // 298.5
  EXPECT_EQ(percentOf(1, 50), 1);       // 0.5
  EXPECT_EQ(percentOf(1, 12), 0);       // 0.12
  EXPECT_EQ(percentOf(8999, 15), 1350); // 1349.85
}

TEST(Money, PercentOfValidatesInput) {
  EXPECT_THROW(percentOf(-1, 10), std::invalid_argument);
  EXPECT_THROW(percentOf(100, 101), std::invalid_argument);
}

TEST(Money, FormatsCents) {
  EXPECT_EQ(formatCents(1234), std::string("$12.34"));
  EXPECT_EQ(formatCents(5), std::string("$0.05"));
  EXPECT_EQ(formatCents(0), std::string("$0.00"));
  EXPECT_EQ(formatCents(-150), std::string("-$1.50"));
}

// ---------- Strings ----------

TEST(Strings, LowerAndJoin) {
  EXPECT_EQ(toLower("Electronics"), std::string("electronics"));
  EXPECT_EQ(join({"a", "b", "c"}, ", "), std::string("a, b, c"));
  EXPECT_EQ(join({}, ", "), std::string(""));
}
