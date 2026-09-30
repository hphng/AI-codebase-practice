#pragma once

#include <string>
#include <vector>

#include "model/Models.hpp"

namespace returns {

// Total paid back to customers: the sum of Decision::refund over approved decisions. Restocking fees
// are kept by us, so they are NOT part of this total. Denied decisions contribute nothing.
Cents totalRefunded(const std::vector<Decision>& decisions);

// Loads policies.json, customers.json, orders.json and requests.json from `dataDir`,
// processes every request in file order and returns the printable report.
std::string runReport(const std::string& dataDir);

}  // namespace returns
