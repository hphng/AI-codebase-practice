#pragma once

#include <map>
#include <set>
#include <string>

#include "json/Json.hpp"
#include "model/Models.hpp"

namespace returns {

struct CategoryPolicy {
  bool returnable = true;
  int windowDays = 30;
  int restockingFeePercent = 0;
};

// Return policy loaded from data/policies.json.
//
// "default" defines the baseline policy. Each entry under "categories" only
// lists what differs from the baseline; any field it omits is inherited from
// "default". Category names are matched case-insensitively, and categories
// with no entry use the default policy.
class PolicyConfig {
 public:
  static PolicyConfig fromJson(const json::Value& root);

  const CategoryPolicy& defaults() const { return defaults_; }
  const CategoryPolicy& forCategory(const std::string& category) const;

  // Extra return-window days granted by loyalty tier (0 if none configured).
  int tierExtraDays(Tier tier) const;

  // Reasons for which no restocking fee is charged (e.g. "defective").
  bool isFeeWaived(const std::string& reason) const;

  // Whether final-sale items may still be returned when defective.
  bool finalSaleAllowsDefective() const { return finalSaleAllowsDefective_; }

  // Abuse limit: at most `velocityMaxReturns()` approved returns per customer
  // within any `velocityWindowDays()`-day span.
  int velocityMaxReturns() const { return velocityMaxReturns_; }
  int velocityWindowDays() const { return velocityWindowDays_; }

 private:
  CategoryPolicy defaults_;
  std::map<std::string, CategoryPolicy> categories_;
  std::map<Tier, int> tierExtraDays_;
  std::set<std::string> feeWaivedReasons_;
  bool finalSaleAllowsDefective_ = false;
  int velocityMaxReturns_ = 5;
  int velocityWindowDays_ = 30;
};

}  // namespace returns
