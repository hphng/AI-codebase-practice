#include "engine/Rules.hpp"

namespace returns {

RuleResult ReturnableCategoryRule::evaluate(const EvalContext& ctx) const {
  if (!ctx.policy.returnable) {
    return RuleResult::deny("category \"" + ctx.item.category + "\" is not returnable");
  }
  return RuleResult::pass();
}

RuleResult FinalSaleRule::evaluate(const EvalContext& ctx) const {
  if (!ctx.item.finalSale) return RuleResult::pass();
  if (allowDefective && ctx.request.reason == "defective") return RuleResult::pass();
  return RuleResult::deny("final-sale items cannot be returned");
}

RuleResult QuantityRule::evaluate(const EvalContext& ctx) const {
  if (ctx.request.quantity < 1) return RuleResult::deny("quantity must be at least 1");
  if (ctx.request.quantity > ctx.item.returnableQuantity()) {
    return RuleResult::deny("only " + std::to_string(ctx.item.returnableQuantity()) +
                            " unit(s) left to return");
  }
  return RuleResult::pass();
}

RuleResult ReturnWindowRule::evaluate(const EvalContext& ctx) const {
  const int window = ctx.config.defaults().windowDays + ctx.config.tierExtraDays(ctx.customer.tier);
  const auto elapsed = daysBetween(ctx.order.purchaseDate, ctx.request.requestDate);

  if (elapsed < 0) return RuleResult::deny("request is dated before the purchase");
  if (elapsed > window) {
    return RuleResult::deny("outside the " + std::to_string(window) + "-day return window");
  }
  return RuleResult::pass();
}

RuleResult VelocityRule::evaluate(const EvalContext& ctx) const {
  const int recent = ctx.velocity.countInWindow(ctx.household, ctx.request.requestDate);
  if (recent >= ctx.config.velocityMaxReturns()) {
    return RuleResult::deny("household already has " + std::to_string(recent) + " returns in the last " +
                            std::to_string(ctx.velocity.windowDays()) + " days");
  }
  return RuleResult::pass();
}

RuleResult RestockingFeeRule::evaluate(const EvalContext& ctx) const {
  if (ctx.request.reason == "defective") return RuleResult::pass();  // we never charge for our own faults
  const Cents gross = ctx.item.unitPrice * ctx.request.quantity;
  return RuleResult::charge(percentOf(gross, ctx.policy.restockingFeePercent));
}

}  // namespace returns
