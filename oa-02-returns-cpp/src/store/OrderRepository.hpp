#pragma once

#include <string>
#include <unordered_map>

#include "model/Models.hpp"

namespace returns {

class OrderRepository {
 public:
  void add(Order order);  // throws std::invalid_argument on a duplicate id
  const Order* find(const std::string& orderId) const;

  // Adds `quantity` to the returned count of the matching line item.
  // Returns false if the order or SKU does not exist.
  bool recordReturn(const std::string& orderId, const std::string& sku, int quantity);

  std::size_t size() const { return orders_.size(); }

 private:
  std::unordered_map<std::string, Order> orders_;
};

}  // namespace returns
