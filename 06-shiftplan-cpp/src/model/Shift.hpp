#pragma once

#include <string>

namespace shiftplan {

// The shift being planned. Planning works in "minutes since the shift started" (0 = shift start);
// the files and the report use clock times ("22:00"). A night shift crosses midnight, so a clock
// time after midnight is later in the shift than one before it.
struct Shift {
  std::string site;   // "SEA8 night outbound"
  int start = 0;      // clock time the shift starts, minutes after midnight (22:00 -> 1320)
  int minutes = 0;    // shift length

  // Clock time (minutes after midnight) -> minutes since the shift started, across midnight.
  // With start 22:00: 23:30 -> 90, 00:30 -> 150, 05:00 -> 420.
  int minutesIntoShift(int clock) const;

  // Minutes since the shift started -> "HH:MM" on the clock. With start 22:00: 150 -> "00:30".
  std::string clockAt(int minutesIn) const;
};

}  // namespace shiftplan
