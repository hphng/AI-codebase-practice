#include "store/CustomerRepository.hpp"

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

}  // namespace returns
