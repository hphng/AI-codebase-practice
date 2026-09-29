#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "util/Date.hpp"

namespace returns {

// Tracks approved returns per key (the service uses household ids) to detect return abuse.
//
// Returns can be recorded in any order (batches are replayed, late files arrive),
// so lookups must not assume chronological recording.
class ReturnVelocityTracker {
 public:
  explicit ReturnVelocityTracker(int windowDays) : windowDays_(windowDays) {}

  void record(const std::string& customerId, const Date& date);

  // Number of recorded returns for `customerId` dated inside the `windowDays`-day
  // window that ends on `date` (inclusive): (date - windowDays, date].
  int countInWindow(const std::string& customerId, const Date& date) const;

  int windowDays() const { return windowDays_; }

 private:
  int windowDays_;
  // customerId -> return dates as day numbers, ascending, so lookups can binary-search.
  std::unordered_map<std::string, std::vector<std::int64_t>> days_;
};

}  // namespace returns
