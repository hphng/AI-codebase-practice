#pragma once

// Builders shared by the engine / service tests.

#include <string>

#include "json/Json.hpp"
#include "model/Models.hpp"
#include "policy/PolicyConfig.hpp"

namespace testdata {

using namespace returns;

// Every category spells out all of its fields, so these tests don't depend on inheritance.
inline PolicyConfig standardConfig() {
  return PolicyConfig::fromJson(json::parse(R"({
    "default":    { "returnable": true,  "windowDays": 30, "restockingFeePercent": 0 },
    "categories": {
      "electronics": { "returnable": true,  "windowDays": 30, "restockingFeePercent": 15 },
      "apparel":     { "returnable": true,  "windowDays": 60, "restockingFeePercent": 0 },
      "grocery":     { "returnable": false, "windowDays": 0,  "restockingFeePercent": 0 }
    },
    "tierExtraDays": { "prime": 15, "business": 30 },
    "feeWaivedReasons": ["defective", "wrong_item"],
    "finalSale": { "allowDefective": true },
    "velocity": { "maxReturns": 3, "windowDays": 10 }
  })"));
}

inline LineItem item(std::string sku, std::string category, Cents price, int quantity, bool finalSale = false) {
  LineItem li;
  li.sku = sku;
  li.name = sku;
  li.category = std::move(category);
  li.unitPrice = price;
  li.quantity = quantity;
  li.finalSale = finalSale;
  return li;
}

inline Order order(std::string id, std::string customerId, const char* date, std::vector<LineItem> items) {
  Order o;
  o.id = std::move(id);
  o.customerId = std::move(customerId);
  o.purchaseDate = Date::parse(date);
  o.items = std::move(items);
  return o;
}

inline Customer customer(std::string id, Tier tier = Tier::Standard) {
  Customer c;
  c.id = id;
  c.name = id;
  c.tier = tier;
  return c;
}

inline ReturnRequest request(std::string id, std::string orderId, std::string sku, int quantity,
                             const char* date, std::string reason = "changed_mind") {
  ReturnRequest r;
  r.id = std::move(id);
  r.orderId = std::move(orderId);
  r.sku = std::move(sku);
  r.quantity = quantity;
  r.requestDate = Date::parse(date);
  r.reason = std::move(reason);
  return r;
}

}  // namespace testdata
