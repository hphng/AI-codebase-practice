#include "store/OrderRepository.hpp"

#include <stdexcept>

namespace returns {

void OrderRepository::add(Order order) {
  const std::string id = order.id;
  if (!orders_.emplace(id, std::move(order)).second) {
    throw std::invalid_argument("duplicate order id " + id);
  }
}

const Order* OrderRepository::find(const std::string& orderId) const {
  auto it = orders_.find(orderId);
  return it == orders_.end() ? nullptr : &it->second;
}

bool OrderRepository::recordReturn(const std::string& orderId, const std::string& sku, int quantity) {
  auto it = orders_.find(orderId);
  if (it == orders_.end()) return false;

  for (auto item : it->second.items) {
    if (item.sku == sku) {
      item.returnedQuantity += quantity;
      return true;
    }
  }
  return false;
}

}  // namespace returns
