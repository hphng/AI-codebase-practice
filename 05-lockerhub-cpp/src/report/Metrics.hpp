#pragma once

#include <optional>
#include <vector>

#include "station/ParcelRecord.hpp"

namespace lockerhub {

// Pickup rate, in percent: of the parcels whose stay at the station is over (picked up or returned
// to sender), the share that the customer picked up. Parcels still in a locker or on the waitlist
// have no outcome yet, so they are left out entirely. 0.0 if no parcel has an outcome yet.
double pickupRatePercent(const std::vector<ParcelRecord>& records);

// Average dwell time of picked-up parcels: minutes from being placed in a compartment to being
// picked up, rounded down to a whole minute. std::nullopt if nothing was picked up.
std::optional<int> averageDwellMinutes(const std::vector<ParcelRecord>& records);

}  // namespace lockerhub
