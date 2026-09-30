#pragma once

#include <queue>
#include <vector>

#include "model/Parcel.hpp"

namespace vanroute {

// Hands out parcels in dispatch order (ops rule DSP-7):
//   1. earliest deadline first;
//   2. same deadline: larger parcels first, so bulky items get van space before small ones;
//   3. same deadline and size: parcel id, A to Z.
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
      // priority_queue pops the "largest" element, so each rule is written as "a comes after b".
      if (a.deadline != b.deadline) return a.deadline > b.deadline;
      return a.id > b.id;
    }
  };

  std::priority_queue<Parcel, std::vector<Parcel>, DispatchOrder> heap_;
};

}  // namespace vanroute
