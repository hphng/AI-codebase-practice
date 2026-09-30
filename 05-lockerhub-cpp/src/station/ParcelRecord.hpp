#pragma once

#include <optional>
#include <string>

#include "model/Size.hpp"

namespace lockerhub {

// Where a parcel is in its life at the station.
//   Waiting:   on the waitlist, no compartment yet
//   InLocker:  in a compartment, waiting for the customer
//   PickedUp:  collected by the customer (final)
//   Returned:  not collected in time, handed back to the carrier (final)
enum class ParcelState { Waiting, InLocker, PickedUp, Returned };

// One parcel's history for the day, as recorded by StationDay. The notification, billing and
// reporting modules only ever read these records.
struct ParcelRecord {
  std::string id;
  Size size = Size::Small;
  ParcelState state = ParcelState::Waiting;
  std::string compartment;        // last compartment it was placed in ("" if never placed)
  std::optional<int> placedAt;    // when it went into a compartment
  std::optional<int> endedAt;     // when it was picked up or returned to sender
  bool waitlisted = false;        // spent time on the waitlist
};

}  // namespace lockerhub
