#include "model/Models.hpp"

#include <stdexcept>

#include "util/Strings.hpp"

namespace returns {

Tier parseTier(const std::string& text) {
  const std::string t = toLower(text);
  if (t == "standard") return Tier::Standard;
  if (t == "prime") return Tier::Prime;
  if (t == "business") return Tier::Business;
  throw std::invalid_argument("unknown tier \"" + text + "\"");
}

std::string toString(Tier tier) {
  switch (tier) {
    case Tier::Standard: return "standard";
    case Tier::Prime: return "prime";
    case Tier::Business: return "business";
  }
  return "standard";
}

const LineItem* Order::findItem(const std::string& sku) const {
  for (const auto& item : items) {
    if (item.sku == sku) return &item;
  }
  return nullptr;
}

Decision Decision::deny(std::string requestId, std::string reason) {
  Decision d;
  d.requestId = std::move(requestId);
  d.reasons.push_back(std::move(reason));
  return d;
}

}  // namespace returns
