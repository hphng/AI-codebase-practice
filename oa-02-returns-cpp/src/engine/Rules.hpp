#pragma once

#include "engine/Rule.hpp"

namespace returns {

// Denies items whose category policy is marked non-returnable (e.g. grocery).
class ReturnableCategoryRule : public Rule {
 public:
  std::string name() const override { return "category"; }
  RuleResult evaluate(const EvalContext& ctx) const override;
};

// Denies final-sale items, except defective ones when `allowDefective` is set.
class FinalSaleRule : public Rule {
 public:
  bool allowDefective = false;

  std::string name() const override { return "final-sale"; }
  RuleResult evaluate(const EvalContext& ctx) const override;
};

// The requested quantity must be >= 1 and no more than the units not yet returned.
class QuantityRule : public Rule {
 public:
  std::string name() const override { return "quantity"; }
  RuleResult evaluate(const EvalContext& ctx) const override;
};

// The return window is the category's windowDays plus the customer's tier bonus.
// A return is accepted through the last day of the window: with a 30-day window,
// an item bought on Jan 1 can be returned through Jan 31.
class ReturnWindowRule : public Rule {
 public:
  std::string name() const override { return "return-window"; }
  RuleResult evaluate(const EvalContext& ctx) const override;
};

// Denies the return if the customer already has the maximum number of approved
// returns inside the velocity window ending on the request date.
class VelocityRule : public Rule {
 public:
  std::string name() const override { return "velocity"; }
  RuleResult evaluate(const EvalContext& ctx) const override;
};

// Charges the category's restocking fee on (unit price x quantity), unless the
// reason is on the fee-waived list.
class RestockingFeeRule : public Rule {
 public:
  std::string name() const override { return "restocking-fee"; }
  RuleResult evaluate(const EvalContext& ctx) const override;
};

}  // namespace returns
