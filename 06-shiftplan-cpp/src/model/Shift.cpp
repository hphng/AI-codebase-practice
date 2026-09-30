#include "model/Shift.hpp"

#include "io/Clock.hpp"

namespace shiftplan {

int Shift::minutesIntoShift(int clock) const {
  return (clock - start + kMinutesPerDay) % kMinutesPerDay;
}

std::string Shift::clockAt(int minutesIn) const {
  return formatClock(start + minutesIn);
}

}  // namespace shiftplan
