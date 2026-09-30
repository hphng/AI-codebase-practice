#pragma once

// Builders for hand-made ParcelRecords, so the notification, billing and metrics tests don't need a
// LockerBank.

#include <string>

#include "station/ParcelRecord.hpp"

namespace testrecords {

using lockerhub::ParcelRecord;
using lockerhub::ParcelState;

inline ParcelRecord inLocker(const std::string& id, int placedAt) {
  ParcelRecord r;
  r.id = id;
  r.state = ParcelState::InLocker;
  r.compartment = "A01";
  r.placedAt = placedAt;
  return r;
}

inline ParcelRecord pickedUp(const std::string& id, int placedAt, int pickedUpAt) {
  ParcelRecord r = inLocker(id, placedAt);
  r.state = ParcelState::PickedUp;
  r.endedAt = pickedUpAt;
  return r;
}

inline ParcelRecord returnedToSender(const std::string& id, int placedAt, int returnedAt) {
  ParcelRecord r = inLocker(id, placedAt);
  r.state = ParcelState::Returned;
  r.endedAt = returnedAt;
  return r;
}

inline ParcelRecord waiting(const std::string& id) {
  ParcelRecord r;
  r.id = id;
  r.state = ParcelState::Waiting;
  r.waitlisted = true;
  return r;
}

}  // namespace testrecords
