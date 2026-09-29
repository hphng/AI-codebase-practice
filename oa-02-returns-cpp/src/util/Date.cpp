#include "util/Date.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace returns {

bool isLeapYear(int year) {
  return year % 4 == 0 && year % 100 != 0;
}

int daysInMonth(int year, int month) {
  static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month < 1 || month > 12) {
    throw std::invalid_argument("month out of range: " + std::to_string(month));
  }
  return (month == 2 && isLeapYear(year)) ? 29 : kDays[month - 1];
}

Date Date::parse(const std::string& text) {
  const auto bad = [&text](const std::string& why) {
    return std::invalid_argument("invalid date \"" + text + "\": " + why);
  };

  if (text.size() != 10 || text[4] != '-' || text[7] != '-') throw bad("expected YYYY-MM-DD");
  for (std::size_t i : {0, 1, 2, 3, 5, 6, 8, 9}) {
    if (!std::isdigit(static_cast<unsigned char>(text[i]))) throw bad("expected digits");
  }

  Date d;
  d.year = std::stoi(text.substr(0, 4));
  d.month = std::stoi(text.substr(5, 2));
  d.day = std::stoi(text.substr(8, 2));

  if (d.year < 1970) throw bad("years before 1970 are not supported");
  if (d.month < 1 || d.month > 12) throw bad("no such month");
  if (d.day < 1 || d.day > daysInMonth(d.year, d.month)) throw bad("no such day in that month");
  return d;
}

std::int64_t Date::toDays() const {
  std::int64_t days = 0;
  for (int y = 1970; y < year; ++y) days += isLeapYear(y) ? 366 : 365;
  for (int m = 1; m < month; ++m) days += daysInMonth(year, m);
  return days + (day - 1);
}

std::string Date::toString() const {
  std::ostringstream out;
  out << std::setfill('0') << std::setw(4) << year << '-' << std::setw(2) << month << '-'
      << std::setw(2) << day;
  return out.str();
}

std::int64_t daysBetween(const Date& from, const Date& to) {
  return to.toDays() - from.toDays();
}

}  // namespace returns
