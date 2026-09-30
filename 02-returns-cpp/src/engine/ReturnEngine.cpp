#include "engine/ReturnEngine.hpp"

#include "engine/Rules.hpp"

namespace returns {

Decision ReturnEngine::evaluate(const EvalContext& ctx) const {
  Decision decision;
  decision.requestId = ctx.request.id;

  for (const auto& rule : rules_) {
    const RuleResult result = rule->evaluate(ctx);
    if (result.outcome == Outcome::Deny) {
      decision.reasons.push_back(rule->name() + ": " + result.reason);
      return decision;
    }
    decision.restockingFee += result.fee;
  }

  const Cents gross = ctx.item.unitPrice * ctx.request.quantity;
  decision.approved = true;
  decision.refund = gross - decision.restockingFee;
  return decision;
}

std::vector<std::unique_ptr<Rule>> makeDefaultRules(const PolicyConfig& config) {
  std::vector<std::unique_ptr<Rule>> rules;

  rules.push_back(std::make_unique<ReturnableCategoryRule>());

  FinalSaleRule finalSale;
  finalSale.allowDefective = config.finalSaleAllowsDefective();
  rules.push_back(std::make_unique<FinalSaleRule>(finalSale));

  rules.push_back(std::make_unique<QuantityRule>());
  rules.push_back(std::make_unique<ReturnWindowRule>());
  rules.push_back(std::make_unique<VelocityRule>());
  rules.push_back(std::make_unique<RestockingFeeRule>());

  return rules;
}

}  // namespace returns
