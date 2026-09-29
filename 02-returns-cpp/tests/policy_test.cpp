#include "TestData.hpp"
#include "test_framework.hpp"

using namespace returns;

TEST(Policy, CategoryLookupIsCaseInsensitive) {
  const PolicyConfig config = testdata::standardConfig();
  EXPECT_EQ(config.forCategory("Electronics").restockingFeePercent, 15);
  EXPECT_EQ(config.forCategory("APPAREL").windowDays, 60);
}

TEST(Policy, UnknownCategoryUsesDefault) {
  const PolicyConfig config = testdata::standardConfig();
  EXPECT_EQ(config.forCategory("garden").windowDays, 30);
  EXPECT_EQ(config.forCategory("garden").returnable, true);
}

TEST(Policy, CategoriesInheritOmittedFieldsFromDefault) {
  const PolicyConfig config = PolicyConfig::fromJson(json::parse(R"({
    "default": { "windowDays": 45, "restockingFeePercent": 10 },
    "categories": {
      "apparel":     { "windowDays": 90 },
      "electronics": { "restockingFeePercent": 20 },
      "grocery":     { "returnable": false }
    }
  })"));

  EXPECT_EQ(config.forCategory("apparel").windowDays, 90);
  EXPECT_EQ(config.forCategory("apparel").restockingFeePercent, 10);
  EXPECT_EQ(config.forCategory("electronics").windowDays, 45);
  EXPECT_EQ(config.forCategory("electronics").restockingFeePercent, 20);
  EXPECT_EQ(config.forCategory("grocery").returnable, false);
  EXPECT_EQ(config.forCategory("grocery").windowDays, 45);
}

TEST(Policy, TierExtrasAndWaivedReasons) {
  const PolicyConfig config = testdata::standardConfig();
  EXPECT_EQ(config.tierExtraDays(Tier::Standard), 0);
  EXPECT_EQ(config.tierExtraDays(Tier::Prime), 15);
  EXPECT_TRUE(config.isFeeWaived("Defective"));
  EXPECT_FALSE(config.isFeeWaived("changed_mind"));
  EXPECT_EQ(config.velocityMaxReturns(), 3);
}
