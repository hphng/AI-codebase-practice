#include "TestData.hpp"
#include "engine/ReturnEngine.hpp"
#include "engine/Rules.hpp"
#include "test_framework.hpp"

using namespace returns;
using namespace testdata;

namespace {

// Holds everything an EvalContext refers to, so tests can tweak one piece at a time.
struct Fixture {
  PolicyConfig config = standardConfig();
  Order ord = order("O-1", "C-1", "2024-01-01", {item("TV", "electronics", 1999, 2), item("SHIRT", "apparel", 2500, 1)});
  Customer cust = customer("C-1");
  ReturnVelocityTracker velocity{30};

  Decision run(const ReturnRequest& req) const {
    const LineItem& li = *ord.findItem(req.sku);
    const EvalContext ctx{ord, li, cust, req, config.forCategory(li.category), config, velocity, cust.id};
    return ReturnEngine(makeDefaultRules(config)).evaluate(ctx);
  }

  RuleResult runRule(const Rule& rule, const ReturnRequest& req) const {
    const LineItem& li = *ord.findItem(req.sku);
    const EvalContext ctx{ord, li, cust, req, config.forCategory(li.category), config, velocity, cust.id};
    return rule.evaluate(ctx);
  }
};

}  // namespace

TEST(ReturnWindow, AcceptsReturnInsideWindow) {
  Fixture f;
  EXPECT_TRUE(f.runRule(ReturnWindowRule{}, request("R", "O-1", "TV", 1, "2024-01-15")).outcome == Outcome::Pass);
}

TEST(ReturnWindow, AcceptsReturnOnLastDayOfWindow) {
  Fixture f;  // bought Jan 1, 30-day window -> last day is Jan 31
  EXPECT_TRUE(f.runRule(ReturnWindowRule{}, request("R", "O-1", "TV", 1, "2024-01-31")).outcome == Outcome::Pass);
}

TEST(ReturnWindow, RejectsReturnAfterWindow) {
  Fixture f;
  const RuleResult r = f.runRule(ReturnWindowRule{}, request("R", "O-1", "TV", 1, "2024-02-01"));
  EXPECT_TRUE(r.outcome == Outcome::Deny);
  EXPECT_CONTAINS(r.reason, "30-day");
}

TEST(ReturnWindow, TierAddsExtraDays) {
  Fixture f;
  f.cust.tier = Tier::Prime;  // 30 + 15
  EXPECT_TRUE(f.runRule(ReturnWindowRule{}, request("R", "O-1", "TV", 1, "2024-02-10")).outcome == Outcome::Pass);
  EXPECT_TRUE(f.runRule(ReturnWindowRule{}, request("R", "O-1", "TV", 1, "2024-02-16")).outcome == Outcome::Deny);
}

TEST(ReturnWindow, UsesTheCategoryWindow) {
  Fixture f;  // apparel has a 60-day window, the default is 30
  EXPECT_TRUE(f.runRule(ReturnWindowRule{}, request("R", "O-1", "SHIRT", 1, "2024-02-20")).outcome == Outcome::Pass);
  EXPECT_TRUE(f.runRule(ReturnWindowRule{}, request("R", "O-1", "SHIRT", 1, "2024-03-02")).outcome == Outcome::Deny);
}

TEST(ReturnWindow, RejectsRequestBeforePurchase) {
  Fixture f;
  EXPECT_TRUE(f.runRule(ReturnWindowRule{}, request("R", "O-1", "TV", 1, "2023-12-31")).outcome == Outcome::Deny);
}

TEST(FinalSale, DeniesFinalSaleItems) {
  Fixture f;
  f.ord.items[0].finalSale = true;
  EXPECT_TRUE(f.runRule(FinalSaleRule{}, request("R", "O-1", "TV", 1, "2024-01-05")).outcome == Outcome::Deny);
}

TEST(FinalSale, AllowsDefectiveWhenConfigured) {
  Fixture f;
  f.ord.items[0].finalSale = true;
  FinalSaleRule rule;
  rule.allowDefective = true;
  EXPECT_TRUE(f.runRule(rule, request("R", "O-1", "TV", 1, "2024-01-05", "defective")).outcome == Outcome::Pass);
}

TEST(Quantity, LimitsToUnitsNotYetReturned) {
  Fixture f;
  f.ord.items[0].returnedQuantity = 1;  // bought 2, 1 already returned
  EXPECT_TRUE(f.runRule(QuantityRule{}, request("R", "O-1", "TV", 1, "2024-01-05")).outcome == Outcome::Pass);
  EXPECT_TRUE(f.runRule(QuantityRule{}, request("R", "O-1", "TV", 2, "2024-01-05")).outcome == Outcome::Deny);
  EXPECT_TRUE(f.runRule(QuantityRule{}, request("R", "O-1", "TV", 0, "2024-01-05")).outcome == Outcome::Deny);
}

TEST(Engine, ApprovesAndChargesRestockingFee) {
  Fixture f;
  const Decision d = f.run(request("R-1", "O-1", "TV", 1, "2024-01-10"));
  ASSERT_TRUE(d.approved);
  EXPECT_EQ(d.restockingFee, 300);  // 15% of 19.99 = 2.9985
  EXPECT_EQ(d.refund, 1699);
}

TEST(Engine, WaivesFeeForDefectiveItems) {
  Fixture f;
  const Decision d = f.run(request("R-1", "O-1", "TV", 2, "2024-01-10", "defective"));
  ASSERT_TRUE(d.approved);
  EXPECT_EQ(d.restockingFee, 0);
  EXPECT_EQ(d.refund, 3998);
}

TEST(Engine, WaivesFeeForEveryConfiguredReason) {
  Fixture f;  // standardConfig waives the fee for "defective" and "wrong_item"
  const Decision d = f.run(request("R-1", "O-1", "TV", 1, "2024-01-10", "wrong_item"));
  ASSERT_TRUE(d.approved);
  EXPECT_EQ(d.restockingFee, 0);
  EXPECT_EQ(d.refund, 1999);
}

TEST(Engine, NonReturnableCategoryIsDenied) {
  Fixture f;
  f.ord.items.push_back(item("MILK", "grocery", 399, 1));
  const Decision d = f.run(request("R-1", "O-1", "MILK", 1, "2024-01-02"));
  EXPECT_FALSE(d.approved);
  ASSERT_EQ(d.reasons.size(), static_cast<std::size_t>(1));
  EXPECT_CONTAINS(d.reasons[0], "category");
}

TEST(Engine, DefaultRulesRejectFinalSaleItems) {
  Fixture f;
  f.ord.items[0].finalSale = true;
  const Decision d = f.run(request("R-1", "O-1", "TV", 1, "2024-01-05", "changed_mind"));
  EXPECT_FALSE(d.approved);
  EXPECT_EQ(d.refund, 0);
}

TEST(Engine, DefaultRulesAllowDefectiveFinalSaleWhenConfigured) {
  Fixture f;
  f.ord.items[0].finalSale = true;
  const Decision d = f.run(request("R-1", "O-1", "TV", 1, "2024-01-05", "defective"));
  EXPECT_TRUE(d.approved);
}

TEST(Engine, FirstDenialStopsEvaluation) {
  Fixture f;
  f.ord.items.push_back(item("MILK", "grocery", 399, 1));
  const Decision d = f.run(request("R-1", "O-1", "MILK", 5, "2025-01-01"));  // category, quantity and window all fail
  EXPECT_FALSE(d.approved);
  EXPECT_EQ(d.reasons.size(), static_cast<std::size_t>(1));
}
