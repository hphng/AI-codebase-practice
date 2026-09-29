#pragma once

#include <queue>
#include <vector>

#include "model/Parcel.hpp"

namespace vanroute {

// Hands out parcels in dispatch order: earliest deadline first; ties by parcel id (A to Z).
class DispatchQueue {
 public:
  void push(Parcel parcel) { heap_.push(std::move(parcel)); }
  bool empty() const { return heap_.empty(); }
  std::size_t size() const { return heap_.size(); }

  // Removes and returns the next parcel. Precondition: !empty().
  Parcel pop();

 private:
  struct DispatchOrder {
    bool operator()(const Parcel& a, const Parcel& b) const {
      if (a.deadline != b.deadline) return a.deadline < b.deadline;
      return a.id > b.id;
    }
  };

  std::priority_queue<Parcel, std::vector<Parcel>, DispatchOrder> heap_;
};

}  // namespace vanroute
