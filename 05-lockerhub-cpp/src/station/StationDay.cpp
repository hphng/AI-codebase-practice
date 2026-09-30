#include "station/StationDay.hpp"

#include <unordered_map>
#include <unordered_set>

#include "io/Clock.hpp"
#include "locker/LockerBank.hpp"

namespace lockerhub {

namespace {

std::string pad(const std::string& text, std::size_t width) {
  return text.size() >= width ? text + " " : text + std::string(width - text.size(), ' ');
}

std::string line(int time, const std::string& what, const std::string& parcel, const std::string& outcome) {
  return formatStamp(time) + "  " + pad(what, 8) + pad(parcel, 10) + "-> " + outcome;
}

class Recorder {
 public:
  Recorder(LockerBank& bank, DayLog& log) : bank_(bank), log_(log) {}

  // Logs what the bank did on its own during the last call: expiries and waitlist moves.
  void settle(int time) {
    const auto returned = bank_.returned();
    for (std::size_t i = returnedSeen_; i < returned.size(); ++i) {
      ParcelRecord& record = current(returned[i]);
      record.state = ParcelState::Returned;
      record.endedAt = time;
      log_.activity.push_back(line(time, "RETURN", record.id, "not collected in time, back to carrier"));
    }
    returnedSeen_ = returned.size();

    const auto nowWaiting = bank_.waiting();
    const std::unordered_set<std::string> stillWaiting(nowWaiting.begin(), nowWaiting.end());
    for (const std::string& id : waiting_) {
      if (stillWaiting.count(id)) continue;
      const auto compartment = bank_.locate(id);
      if (!compartment) continue;
      ParcelRecord& record = current(id);
      record.state = ParcelState::InLocker;
      record.compartment = *compartment;
      record.placedAt = time;
      log_.activity.push_back(line(time, "MOVE IN", id, *compartment + " (from the waitlist)"));
    }
    waiting_ = nowWaiting;
  }

  void deposit(const Event& event) {
    const DepositResult result = bank_.deposit(event.minute, event.parcelId, event.size);
    const std::string label = event.parcelId + " (" + sizeCode(event.size) + ")";
    if (result.status == DepositStatus::Duplicate) {
      log_.activity.push_back(line(event.minute, "DEPOSIT", label, "refused, already at the station"));
      return;
    }

    ParcelRecord record;
    record.id = event.parcelId;
    record.size = event.size;
    if (result.status == DepositStatus::Stored) {
      record.state = ParcelState::InLocker;
      record.compartment = result.compartment;
      record.placedAt = event.minute;
      log_.activity.push_back(line(event.minute, "DEPOSIT", label, result.compartment));
    } else {
      record.state = ParcelState::Waiting;
      record.waitlisted = true;
      log_.activity.push_back(line(event.minute, "DEPOSIT", label, "no space, added to the waitlist"));
    }
    index_[record.id] = log_.records.size();
    log_.records.push_back(record);
  }

  void pickup(const Event& event) {
    if (!bank_.pickup(event.minute, event.parcelId)) {
      log_.activity.push_back(line(event.minute, "PICKUP", event.parcelId, "not in a locker"));
      return;
    }
    ParcelRecord& record = current(event.parcelId);
    record.state = ParcelState::PickedUp;
    record.endedAt = event.minute;
    log_.activity.push_back(line(event.minute, "PICKUP", event.parcelId, "collected from " + record.compartment));
  }

 private:
  ParcelRecord& current(const std::string& id) { return log_.records.at(index_.at(id)); }

  LockerBank& bank_;
  DayLog& log_;
  std::unordered_map<std::string, std::size_t> index_;  // parcel id -> its latest record
  std::size_t returnedSeen_ = 0;
  std::vector<std::string> waiting_;
};

}  // namespace

DayLog runDay(const StationConfig& station, const std::vector<Compartment>& compartments,
              const std::vector<Event>& events) {
  LockerBank bank(compartments, station.holdHours * 60);
  DayLog log;
  Recorder recorder(bank, log);

  for (const Event& event : events) {
    bank.advance(event.minute);
    recorder.settle(event.minute);

    switch (event.kind) {
      case EventKind::Deposit:
        recorder.deposit(event);
        break;
      case EventKind::Pickup:
        recorder.pickup(event);
        break;
      case EventKind::Close:
        log.activity.push_back(formatStamp(event.minute) + "  CLOSE");
        break;
    }
    recorder.settle(event.minute);
    log.closedAt = event.minute;
  }

  for (int s = 0; s < kSizeCount; ++s) log.freeAtClose[s] = bank.freeCount(static_cast<Size>(s));
  log.waitlistAtClose = bank.waiting();
  log.longestInLockers = bank.longestWaiting(3);
  return log;
}

}  // namespace lockerhub
