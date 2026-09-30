#pragma once

#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "model/Compartment.hpp"
#include "model/Size.hpp"

namespace lockerhub {

enum class DepositStatus { Stored, NoSpace, Duplicate };

struct DepositResult {
  DepositStatus status = DepositStatus::NoSpace;
  std::string compartment;  // set only when status == Stored
};

inline std::ostream& operator<<(std::ostream& out, DepositStatus status) {
  switch (status) {
    case DepositStatus::Stored:
      return out << "Stored";
    case DepositStatus::NoSpace:
      return out << "NoSpace";
    case DepositStatus::Duplicate:
      return out << "Duplicate";
  }
  return out << "?";
}

// PART A: implement LockerBank (src/locker/LockerBank.cpp).
//
// LockerBank runs one Hub Locker station: it picks the compartment for every parcel a courier drops
// off, frees it when the customer picks the parcel up, sends unclaimed parcels back to the carrier,
// and keeps a waitlist of parcels that didn't fit. StationDay (src/station/) replays the day's event
// feed through it, and the CLI report shows the result.
//
// It is built in four levels. Each level's examples are in tests/locker_level<N>_test.cpp; run one
// level with `unit_tests.exe Level1` (Level2, ...). Later levels add rules; they never change what an
// earlier level's tests expect.
//
// Times are whole minutes since day 1, 00:00. The query methods (the const ones) describe the state
// after the most recent call that takes a time.
//
// ---- Level 1: store and pick up -----------------------------------------------------------------
//  1. The constructor takes the station's compartments (unique ids, in any order) and the hold time
//     in minutes. It throws std::invalid_argument if two compartments share an id or if
//     holdMinutes <= 0. Every compartment starts empty.
//  2. deposit(time, parcelId, size) puts the parcel into a free compartment it fits (see fits() in
//     model/Size.hpp). It uses the SMALLEST size class that has a free fitting compartment, and within
//     that class the compartment with the lowest id (plain string comparison).
//     Returns {Stored, <compartment id>}.
//  3. A parcel id that is already in the bank (in a compartment, or on the waitlist from level 4) is
//     refused: {Duplicate, ""}, and nothing changes.
//  4. If no free compartment fits the parcel: {NoSpace, ""}. (Level 4 says what happens to it.)
//  5. pickup(time, parcelId) empties the parcel's compartment and returns true. For a parcel that
//     isn't in a compartment (unknown, or already picked up) it returns false and changes nothing.
//     Once picked up, the same id may be deposited again later.
//
// ---- Level 2: queries ---------------------------------------------------------------------------
//  6. locate(parcelId): the id of the compartment the parcel is in, or std::nullopt.
//  7. freeCount(size): how many compartments of exactly that size class are empty.
//  8. longestWaiting(k): the ids of the (up to) k parcels that have been in their compartments the
//     longest: earliest placement time first, ties by parcel id ascending. k <= 0 gives an empty list.
//
// ---- Level 3: pickup window ---------------------------------------------------------------------
//  9. A parcel placed into a compartment at time p can be picked up until p + holdMinutes, inclusive.
// 10. Every call that takes a time (deposit, pickup, advance) FIRST expires every parcel whose window
//     has ended (p + holdMinutes < time): its compartment is emptied and the parcel is returned to
//     sender. Only then does the call do its own work, so a deposit can use a compartment emptied this
//     way, and a pickup of an expired parcel returns false.
// 11. returned(): the ids of all parcels returned to sender so far, in the order they expired: end of
//     window ascending, ties by parcel id ascending.
// 12. advance(time) does rule 10 and nothing else. The station calls it at closing time.
// 13. Time never goes backwards. A call whose time is earlier than the previous call's time throws
//     std::invalid_argument and changes nothing. (Equal times are fine.)
//
// ---- Level 4: waitlist, at station scale --------------------------------------------------------
// 14. A deposit that gets NoSpace joins the back of the waitlist (the result is still {NoSpace, ""}).
//     waiting(): the waitlisted ids, front of the queue first. A waitlisted parcel can't be picked up
//     (pickup returns false) and is never returned to sender: its window only starts once it is
//     placed into a compartment.
// 15. Whenever a compartment is emptied (by a pickup, or by an expiry under rule 10) it is offered to
//     the waitlist at once: of the waitlisted parcels that fit it, the one that joined the waitlist
//     EARLIEST is placed into it, with the current call's time as its placement time. If none fits,
//     the compartment stays empty. Expiries are handled one at a time in rule 11's order, and each
//     emptied compartment is offered before the next parcel expires.
// 16. Performance: a large station has up to 50,000 compartments, and a day's feed has up to 200,000
//     calls. Each call must be much cheaper than a pass over all compartments, all stored parcels or
//     the whole waitlist. There is a performance test in tests/locker_level4_test.cpp.
class LockerBank {
 public:
  LockerBank(std::vector<Compartment> compartments, int holdMinutes);
  ~LockerBank();
  LockerBank(LockerBank&&) noexcept;
  LockerBank& operator=(LockerBank&&) noexcept;
  LockerBank(const LockerBank&) = delete;
  LockerBank& operator=(const LockerBank&) = delete;

  // Level 1
  DepositResult deposit(int time, const std::string& parcelId, Size size);
  bool pickup(int time, const std::string& parcelId);

  // Level 2
  std::optional<std::string> locate(const std::string& parcelId) const;
  int freeCount(Size size) const;
  std::vector<std::string> longestWaiting(int k) const;

  // Level 3
  void advance(int time);
  std::vector<std::string> returned() const;

  // Level 4
  std::vector<std::string> waiting() const;

 private:
  // Defined in LockerBank.cpp. Put whatever data your implementation needs in there.
  struct State;
  std::unique_ptr<State> state_;
};

}  // namespace lockerhub
