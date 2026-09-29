#include "dispatch/DispatchQueue.hpp"

namespace vanroute {

Parcel DispatchQueue::pop() {
  Parcel next = heap_.top();
  heap_.pop();
  return next;
}

}  // namespace vanroute
