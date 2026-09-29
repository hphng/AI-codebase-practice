#pragma once

#include <string>

#include "fraud/ReturnVelocityTracker.hpp"
#include "model/Models.hpp"
#include "policy/PolicyConfig.hpp"

namespace returns {

// Everything a rule may look at when judging one return request.
struct EvalContext {
  const Order& order;
  const LineItem& item;
  const Customer& customer;
  const ReturnRequest& request;
  const CategoryPolicy& policy;
  const PolicyConfig& config;
  const ReturnVelocityTracker& velocity;
};

enum class Outcome { Pass, Deny };

struct RuleResult {
  Outcome outcome = Outcome::Pass;
  std::string reason;  // set when denied
  Cents fee = 0;       // restocking fee this rule charges (only when passing)

  static RuleResult pass() { return {}; }
  static RuleResult deny(std::string why) { return {Outcome::Deny, std::move(why), 0}; }
  static RuleResult charge(Cents fee) { return {Outcome::Pass, "", fee}; }
};

// Base class for return rules. The default implementation has no opinion
// (it passes), so a rule only overrides what it cares about.
class Rule {
 public:
  virtual ~Rule() = default;
  virtual std::string name() const { return "rule"; }
  virtual RuleResult evaluate(const EvalContext& ctx) const {
    (void)ctx;
    return RuleResult::pass();
  }
};

}  // namespace returns
