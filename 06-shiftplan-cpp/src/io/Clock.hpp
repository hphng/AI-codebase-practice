#pragma once

#include <string>

namespace shiftplan {

constexpr int kMinutesPerDay = 24 * 60;

// "HH:MM" (24-hour) -> minutes after midnight. Throws std::invalid_argument on anything else.
int parseClock(const std::string& text);

// Minutes after midnight -> "HH:MM". Values past midnight wrap: formatClock(1500) == "01:00".
std::string formatClock(int minutes);

}  // namespace shiftplan
