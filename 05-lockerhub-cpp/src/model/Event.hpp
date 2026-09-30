#pragma once

#include <string>

#include "model/Size.hpp"

namespace lockerhub {

// One line of the station's event feed (data/events.csv).
//   Deposit: a courier drops off parcelId (size is the parcel's size class).
//   Pickup:  a customer collects parcelId.
//   Close:   the station closes for the day. parcelId is empty.
enum class EventKind { Deposit, Pickup, Close };

struct Event {
  int minute = 0;  // minutes since day 1, 00:00 (see io/Clock.hpp)
  EventKind kind = EventKind::Deposit;
  std::string parcelId;
  Size size = Size::Small;  // only meaningful for Deposit
};

}  // namespace lockerhub
