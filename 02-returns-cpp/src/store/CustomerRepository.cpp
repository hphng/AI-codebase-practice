#include "store/CustomerRepository.hpp"

#include <algorithm>
#include <stdexcept>

namespace returns {

void CustomerRepository::add(Customer customer) {
  const std::string id = customer.id;
  if (!customers_.emplace(id, std::move(customer)).second) {
    throw std::invalid_argument("duplicate customer id " + id);
  }
}

const Customer* CustomerRepository::find(const std::string& customerId) const {
  auto it = customers_.find(customerId);
  return it == customers_.end() ? nullptr : &it->second;
}

std::vector<Customer> CustomerRepository::all() const {
  std::vector<Customer> out;
  out.reserve(customers_.size());
  for (const auto& entry : customers_) out.push_back(entry.second);
  std::sort(out.begin(), out.end(), [](const Customer& a, const Customer& b) { return a.id < b.id; });
  return out;
}

}  // namespace returns
