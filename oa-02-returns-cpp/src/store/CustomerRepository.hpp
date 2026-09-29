#pragma once

#include <string>
#include <unordered_map>

#include "model/Models.hpp"

namespace returns {

class CustomerRepository {
 public:
  void add(Customer customer);  // throws std::invalid_argument on a duplicate id
  const Customer* find(const std::string& customerId) const;
  std::size_t size() const { return customers_.size(); }

 private:
  std::unordered_map<std::string, Customer> customers_;
};

}  // namespace returns
