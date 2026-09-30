#pragma once

#include <string>

namespace lockerhub {

// The station counts time in whole minutes since day 1, 00:00.
//   parseStamp("D1", "08:00") == 480
//   parseStamp("D2", "00:30") == 1470
// Days are "D1".."D99", times "HH:MM" (24-hour). Throws std::invalid_argument on anything else.
int parseStamp(const std::string& day, const std::string& time);

// Inverse of parseStamp: formatStamp(1470) == "D2 00:30".
std::string formatStamp(int minute);

// A length of time: formatDuration(1265) == "21h 05m", formatDuration(45) == "0h 45m".
std::string formatDuration(int minutes);

}  // namespace lockerhub
