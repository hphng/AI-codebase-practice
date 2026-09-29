#pragma once

#include <string>
#include <vector>

#include "util/Date.hpp"
#include "util/Money.hpp"

namespace returns {

enum class Tier { Standard, Prime, Business };

Tier parseTier(const std::string& text);  // case-insensitive; throws std::invalid_argument
std::string toString(Tier tier);

struct Customer {
  std::string id;
  std::string name;
  Tier tier = Tier::Standard;
  std::vector<std::string> addresses;            // shipping addresses on file
  std::vector<std::string> paymentFingerprints;  // hashed card identifiers
};

struct LineItem {
  std::string sku;
  std::string name;
  std::string category;
  Cents unitPrice = 0;
  int quantity = 0;
  int returnedQuantity = 0;  // units already refunded
  bool finalSale = false;

  int returnableQuantity() const { return quantity - returnedQuantity; }
};

struct Order {
  std::string id;
  std::string customerId;
  Date purchaseDate;
  std::vector<LineItem> items;

  const LineItem* findItem(const std::string& sku) const;
};

struct ReturnRequest {
  std::string id;
  std::string orderId;
  std::string sku;
  int quantity = 1;
  Date requestDate;
  std::string reason;  // e.g. "defective", "changed_mind", "wrong_item"
};

struct Decision {
  std::string requestId;
  bool approved = false;
  Cents refund = 0;
  Cents restockingFee = 0;
  std::vector<std::string> reasons;  // why it was denied

  static Decision deny(std::string requestId, std::string reason);
};

}  // namespace returns
