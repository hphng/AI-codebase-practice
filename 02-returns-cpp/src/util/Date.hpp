#pragma once

#include <cstdint>
#include <string>

namespace returns {

bool isLeapYear(int year);

// Number of days in `month` (1-12) of `year`.
int daysInMonth(int year, int month);

struct Date {
  int year = 1970;
  int month = 1;
  int day = 1;

  // Parses "YYYY-MM-DD". Throws std::invalid_argument on a bad format or an impossible date.
  static Date parse(const std::string& text);

  // Days since 1970-01-01. Dates before 1970 are not supported.
  std::int64_t toDays() const;

  std::string toString() const;

  friend bool operator==(const Date& a, const Date& b) {
    return a.year == b.year && a.month == b.month && a.day == b.day;
  }
  friend bool operator!=(const Date& a, const Date& b) { return !(a == b); }
  friend bool operator<(const Date& a, const Date& b) { return a.toDays() < b.toDays(); }
};

// Signed number of days from `from` to `to`, i.e. to - from.
std::int64_t daysBetween(const Date& from, const Date& to);

}  // namespace returns
