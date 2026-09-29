#include "fraud/ReturnVelocityTracker.hpp"

#include <algorithm>

namespace returns {

void ReturnVelocityTracker::record(const std::string& customerId, const Date& date) {
  auto& days = days_[customerId];
  const std::int64_t day = date.toDays();
  days.insert(std::upper_bound(days.begin(), days.end(), day), day);
}

int ReturnVelocityTracker::countInWindow(const std::string& customerId, const Date& date) const {
  auto it = days_.find(customerId);
  if (it == days_.end()) return 0;

  const std::vector<std::int64_t>& days = it->second;
  const std::int64_t last = date.toDays();
  const std::int64_t first = last - windowDays_ + 1;

  auto lo = std::lower_bound(days.begin(), days.end(), first);
  auto hi = std::upper_bound(days.begin(), days.end(), last);
  return static_cast<int>(hi - lo);
}

}  // namespace returns
