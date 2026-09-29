#pragma once

#include <string>

namespace returns {

// Loads policies.json, customers.json, orders.json and requests.json from `dataDir`,
// processes every request in file order and returns the printable report.
std::string runReport(const std::string& dataDir);

}  // namespace returns
