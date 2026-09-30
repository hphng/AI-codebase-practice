#pragma once

#include <string>
#include <vector>

#include "station/ParcelRecord.hpp"

namespace lockerhub {

// Hub partner payout (finance rule FIN-4). The store that hosts the lockers is paid for every parcel
// that leaves the station:
//   - 40 cents when the customer picks it up,
//   - 25 cents when it is returned to sender (the partner hands it back to the carrier).
// Parcels still in a locker or on the waitlist are paid on the day they leave.
constexpr long long kPickupPayoutCents = 40;
constexpr long long kReturnPayoutCents = 25;

// The day's payout in cents, per the rule above.
long long partnerPayoutCents(const std::vector<ParcelRecord>& records);

// 650 -> "$6.50", 5 -> "$0.05".
std::string formatMoney(long long cents);

}  // namespace lockerhub
