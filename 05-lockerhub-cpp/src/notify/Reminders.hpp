#pragma once

#include <string>
#include <vector>

#include "station/ParcelRecord.hpp"

namespace lockerhub {

// Customer notifications (ops rule NTF-2): a customer gets a pickup reminder once their parcel has
// been in its locker for 24 hours or more.
constexpr int kReminderAfterMinutes = 24 * 60;

// The parcels due a reminder at time `now`: parcels in a locker (state InLocker) that were placed
// kReminderAfterMinutes or more before `now`. Earliest placed first, ties by parcel id. Parcels on
// the waitlist, picked up or returned never get one.
std::vector<std::string> dueReminders(const std::vector<ParcelRecord>& records, int now);

}  // namespace lockerhub
