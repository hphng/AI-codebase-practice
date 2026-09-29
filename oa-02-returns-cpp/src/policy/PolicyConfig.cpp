#include "policy/PolicyConfig.hpp"

#include "util/Strings.hpp"

namespace returns {

namespace {

// Reads the fields present in `value`; anything missing keeps the value from `base`.
CategoryPolicy parseCategory(const json::Value& value, const CategoryPolicy& base) {
  CategoryPolicy policy = base;
  policy.returnable = value.getBool("returnable", base.returnable);
  policy.windowDays = static_cast<int>(value.getInt("windowDays", base.windowDays));
  policy.restockingFeePercent =
      static_cast<int>(value.getInt("restockingFeePercent", base.restockingFeePercent));
  return policy;
}

}  // namespace

PolicyConfig PolicyConfig::fromJson(const json::Value& root) {
  PolicyConfig config;

  if (root.has("default")) {
    config.defaults_ = parseCategory(root["default"], CategoryPolicy{});
  }

  if (root.has("categories")) {
    for (const auto& [name, value] : root["categories"].asObject()) {
      config.categories_[toLower(name)] = parseCategory(value, CategoryPolicy{});
    }
  }

  if (root.has("tierExtraDays")) {
    for (const auto& [tier, days] : root["tierExtraDays"].asObject()) {
      config.tierExtraDays_[parseTier(tier)] = static_cast<int>(days.asInt());
    }
  }

  if (root.has("feeWaivedReasons")) {
    for (const auto& reason : root["feeWaivedReasons"].asArray()) {
      config.feeWaivedReasons_.insert(toLower(reason.asString()));
    }
  }

  if (root.has("finalSale")) {
    config.finalSaleAllowsDefective_ = root["finalSale"].getBool("allowDefective", false);
  }

  if (root.has("velocity")) {
    const auto& v = root["velocity"];
    config.velocityMaxReturns_ = static_cast<int>(v.getInt("maxReturns", config.velocityMaxReturns_));
    config.velocityWindowDays_ = static_cast<int>(v.getInt("windowDays", config.velocityWindowDays_));
  }

  return config;
}

const CategoryPolicy& PolicyConfig::forCategory(const std::string& category) const {
  auto it = categories_.find(toLower(category));
  return it != categories_.end() ? it->second : defaults_;
}

int PolicyConfig::tierExtraDays(Tier tier) const {
  auto it = tierExtraDays_.find(tier);
  return it != tierExtraDays_.end() ? it->second : 0;
}

bool PolicyConfig::isFeeWaived(const std::string& reason) const {
  return feeWaivedReasons_.count(toLower(reason)) > 0;
}

}  // namespace returns
