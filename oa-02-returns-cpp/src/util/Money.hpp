#pragma once

#include <cstdint>
#include <string>

namespace returns {

// All money is handled as integer cents to avoid floating-point drift.
using Cents = std::int64_t;

// `percent`% of `amount`, rounded half-up to the nearest cent.
// Requires amount >= 0 and 0 <= percent <= 100.
// Examples: percentOf(1999, 15) == 300 (299.85), percentOf(1, 50) == 1 (0.5).
Cents percentOf(Cents amount, int percent);

// "$12.34", "$0.05", "-$1.50"
std::string formatCents(Cents amount);

}  // namespace returns
