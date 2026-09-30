#include "report/Metrics.hpp"

namespace lockerhub {

double pickupRatePercent(const std::vector<ParcelRecord>& records) {
  int pickedUp = 0;
  int total = 0;
  for (const ParcelRecord& r : records) {
    ++total;  // every parcel the station took in counts towards the rate
    if (r.state == ParcelState::PickedUp) ++pickedUp;
  }
  if (total == 0) return 0.0;
  return 100.0 * pickedUp / total;
}

std::optional<int> averageDwellMinutes(const std::vector<ParcelRecord>& records) {
  long long total = 0;
  int count = 0;
  for (const ParcelRecord& r : records) {
    if (r.state != ParcelState::PickedUp || !r.placedAt || !r.endedAt) continue;
    total += *r.endedAt - *r.placedAt;
    ++count;
  }
  if (count == 0) return std::nullopt;
  return static_cast<int>(total / count);
}

}  // namespace lockerhub
