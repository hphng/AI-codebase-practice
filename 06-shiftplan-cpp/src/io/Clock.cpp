#include "io/Clock.hpp"

#include <cctype>
#include <stdexcept>

namespace shiftplan {

namespace {

std::string twoDigits(int n) {
  return (n < 10 ? "0" : "") + std::to_string(n);
}

}  // namespace

int parseClock(const std::string& text) {
  const bool shape = text.size() == 5 && text[2] == ':' && std::isdigit(static_cast<unsigned char>(text[0])) &&
                     std::isdigit(static_cast<unsigned char>(text[1])) &&
                     std::isdigit(static_cast<unsigned char>(text[3])) &&
                     std::isdigit(static_cast<unsigned char>(text[4]));
  if (!shape) throw std::invalid_argument("bad clock time '" + text + "' (expected HH:MM)");
  const int hours = std::stoi(text.substr(0, 2));
  const int minutes = std::stoi(text.substr(3));
  if (hours > 23 || minutes > 59) throw std::invalid_argument("bad clock time '" + text + "'");
  return hours * 60 + minutes;
}

std::string formatClock(int minutes) {
  const int inDay = ((minutes % kMinutesPerDay) + kMinutesPerDay) % kMinutesPerDay;
  return twoDigits(inDay / 60) + ":" + twoDigits(inDay % 60);
}

}  // namespace shiftplan
