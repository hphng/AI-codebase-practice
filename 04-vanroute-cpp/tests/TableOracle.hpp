#pragma once

// Test double: travel times from a fixed table instead of the road network.
// Lookups are symmetric; unknown pairs are unreachable (-1); a node to itself is 0.

#include <map>
#include <string>
#include <utility>

#include "dispatch/TravelOracle.hpp"

class TableOracle : public vanroute::TravelOracle {
 public:
  TableOracle& set(const std::string& a, const std::string& b, int minutes) {
    table_[{a, b}] = minutes;
    table_[{b, a}] = minutes;
    return *this;
  }

  int minutes(const std::string& from, const std::string& to) const override {
    if (from == to) return 0;
    auto it = table_.find({from, to});
    return it == table_.end() ? -1 : it->second;
  }

 private:
  std::map<std::pair<std::string, std::string>, int> table_;
};
