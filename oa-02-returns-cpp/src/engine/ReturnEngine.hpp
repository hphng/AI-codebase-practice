#pragma once

#include <memory>
#include <vector>

#include "engine/Rule.hpp"

namespace returns {

// Runs rules in order. The first denial wins and stops evaluation;
// otherwise the return is approved and fees from all rules are summed.
class ReturnEngine {
 public:
  explicit ReturnEngine(std::vector<std::unique_ptr<Rule>> rules) : rules_(std::move(rules)) {}

  Decision evaluate(const EvalContext& ctx) const;

  const std::vector<std::unique_ptr<Rule>>& rules() const { return rules_; }

 private:
  std::vector<std::unique_ptr<Rule>> rules_;
};

// The production rule set, in evaluation order.
std::vector<std::unique_ptr<Rule>> makeDefaultRules(const PolicyConfig& config);

}  // namespace returns
