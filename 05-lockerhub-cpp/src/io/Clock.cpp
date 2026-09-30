#include "io/Clock.hpp"

#include <cctype>
#include <stdexcept>

namespace lockerhub {

namespace {

constexpr int kMinutesPerDay = 24 * 60;

bool allDigits(const std::string& s) {
  if (s.empty()) return false;
  for (char c : s) {
    if (!std::isdigit(static_cast<unsigned char>(c))) return false;
  }
  return true;
}

std::string twoDigits(int n) {
  return (n < 10 ? "0" : "") + std::to_string(n);
}

}  // namespace

int parseStamp(const std::string& day, const std::string& time) {
  if (day.size() < 2 || day.size() > 3 || day[0] != 'D' || !allDigits(day.substr(1))) {
    throw std::invalid_argument("bad day '" + day + "' (expected D1..D99)");
  }
  const int dayNumber = std::stoi(day.substr(1));
  if (dayNumber < 1) throw std::invalid_argument("bad day '" + day + "' (days start at D1)");

  if (time.size() != 5 || time[2] != ':' || !allDigits(time.substr(0, 2)) || !allDigits(time.substr(3))) {
    throw std::invalid_argument("bad time '" + time + "' (expected HH:MM)");
  }
  const int hours = std::stoi(time.substr(0, 2));
  const int minutes = std::stoi(time.substr(3));
  if (hours > 23 || minutes > 59) throw std::invalid_argument("bad time '" + time + "'");

  return (dayNumber - 1) * kMinutesPerDay + hours * 60 + minutes;
}

std::string formatStamp(int minute) {
  const int day = minute / kMinutesPerDay + 1;
  const int inDay = minute % kMinutesPerDay;
  return "D" + std::to_string(day) + " " + twoDigits(inDay / 60) + ":" + twoDigits(inDay % 60);
}

std::string formatDuration(int minutes) {
  return std::to_string(minutes / 60) + "h " + twoDigits(minutes % 60) + "m";
}

}  // namespace lockerhub
