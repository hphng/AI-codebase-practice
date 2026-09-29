#include "util/Money.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace returns {

Cents percentOf(Cents amount, int percent) {
  if (amount < 0) throw std::invalid_argument("amount must be non-negative");
  if (percent < 0 || percent > 100) throw std::invalid_argument("percent must be in [0, 100]");
  return amount * (percent / 100);
}

std::string formatCents(Cents amount) {
  std::ostringstream out;
  if (amount < 0) {
    out << '-';
    amount = -amount;
  }
  out << '$' << amount / 100 << '.' << std::setfill('0') << std::setw(2) << amount % 100;
  return out.str();
}

}  // namespace returns
