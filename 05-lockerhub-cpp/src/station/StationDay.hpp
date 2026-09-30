#pragma once

#include <array>
#include <string>
#include <vector>

#include "model/Compartment.hpp"
#include "model/Event.hpp"
#include "model/Size.hpp"
#include "model/StationConfig.hpp"
#include "station/ParcelRecord.hpp"

namespace lockerhub {

// Everything that happened at the station during one run of the event feed.
struct DayLog {
  std::vector<std::string> activity;          // human-readable, one line per thing that happened
  std::vector<ParcelRecord> records;          // one per accepted parcel, in arrival order
  int closedAt = 0;                           // time of the last event
  std::array<int, kSizeCount> freeAtClose{};  // empty compartments per size class (S, M, L)
  std::vector<std::string> waitlistAtClose;   // front of the queue first
  std::vector<std::string> longestInLockers;  // up to 3 parcels, longest in a compartment first
};

// Replays the event feed through a LockerBank (src/locker/) and records what happened.
//
// For every event at time t it first advances the bank to t, so parcels whose pickup window ended are
// logged as returned to sender (and any waitlisted parcel that moves into the emptied compartment is
// logged as moving in) before the event itself. Then it applies the event:
//   DEPOSIT  a new record is added (duplicates are logged and ignored)
//   PICKUP   the record becomes PickedUp
//   CLOSE    nothing else; the state after it is what the report shows "at close"
DayLog runDay(const StationConfig& station, const std::vector<Compartment>& compartments,
              const std::vector<Event>& events);

}  // namespace lockerhub
