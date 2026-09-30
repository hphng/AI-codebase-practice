#include "billing/Payout.hpp"

namespace lockerhub {

long long partnerPayoutCents(const std::vector<ParcelRecord>& records) {
  long long total = 0;
  for (const ParcelRecord& r : records) {
    if (r.state == ParcelState::PickedUp) total += kPickupPayoutCents;
  }
  return total;
}

std::string formatMoney(long long cents) {
  const long long rest = cents % 100;
  return "$" + std::to_string(cents / 100) + "." + (rest < 10 ? "0" : "") + std::to_string(rest);
}

}  // namespace lockerhub
