#include "notify/Reminders.hpp"

#include <algorithm>
#include <tuple>

namespace lockerhub {

std::vector<std::string> dueReminders(const std::vector<ParcelRecord>& records, int now) {
  std::vector<const ParcelRecord*> due;
  for (const ParcelRecord& r : records) {
    if (r.state != ParcelState::InLocker || !r.placedAt) continue;
    if (now - *r.placedAt > kReminderAfterMinutes) due.push_back(&r);
  }

  std::sort(due.begin(), due.end(), [](const ParcelRecord* a, const ParcelRecord* b) {
    return std::tie(*a->placedAt, a->id) < std::tie(*b->placedAt, b->id);
  });

  std::vector<std::string> ids;
  for (const ParcelRecord* r : due) ids.push_back(r->id);
  return ids;
}

}  // namespace lockerhub
