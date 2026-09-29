#include "model/Loaders.hpp"

namespace returns {

Customer customerFromJson(const json::Value& value) {
  Customer c;
  c.id = value["id"].asString();
  c.name = value.getString("name", "");
  c.tier = parseTier(value.getString("tier", "standard"));
  if (value.has("addresses")) {
    for (const auto& a : value["addresses"].asArray()) c.addresses.push_back(a.asString());
  }
  if (value.has("paymentFingerprints")) {
    for (const auto& p : value["paymentFingerprints"].asArray()) c.paymentFingerprints.push_back(p.asString());
  }
  return c;
}

Order orderFromJson(const json::Value& value) {
  Order o;
  o.id = value["id"].asString();
  o.customerId = value["customerId"].asString();
  o.purchaseDate = Date::parse(value["purchaseDate"].asString());
  for (const auto& raw : value["items"].asArray()) {
    LineItem item;
    item.sku = raw["sku"].asString();
    item.name = raw.getString("name", item.sku);
    item.category = raw["category"].asString();
    item.unitPrice = raw["unitPriceCents"].asInt();
    item.quantity = static_cast<int>(raw["quantity"].asInt());
    item.returnedQuantity = static_cast<int>(raw.getInt("returnedQuantity", 0));
    item.finalSale = raw.getBool("finalSale", false);
    o.items.push_back(item);
  }
  return o;
}

ReturnRequest requestFromJson(const json::Value& value) {
  ReturnRequest r;
  r.id = value["id"].asString();
  r.orderId = value["orderId"].asString();
  r.sku = value["sku"].asString();
  r.quantity = static_cast<int>(value.getInt("quantity", 1));
  r.requestDate = Date::parse(value["requestDate"].asString());
  r.reason = value.getString("reason", "unspecified");
  return r;
}

std::vector<Customer> customersFromJson(const json::Value& array) {
  std::vector<Customer> out;
  for (const auto& v : array.asArray()) out.push_back(customerFromJson(v));
  return out;
}

std::vector<Order> ordersFromJson(const json::Value& array) {
  std::vector<Order> out;
  for (const auto& v : array.asArray()) out.push_back(orderFromJson(v));
  return out;
}

std::vector<ReturnRequest> requestsFromJson(const json::Value& array) {
  std::vector<ReturnRequest> out;
  for (const auto& v : array.asArray()) out.push_back(requestFromJson(v));
  return out;
}

}  // namespace returns
