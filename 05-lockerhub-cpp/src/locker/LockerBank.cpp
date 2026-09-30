#include "locker/LockerBank.hpp"

namespace lockerhub {

// TODO(Part A): replace this placeholder with a real implementation of the spec in LockerBank.hpp.
// Right now the bank keeps no state at all: every deposit is told there is no space, nothing is ever
// stored, picked up, returned or waitlisted, so the station report shows every parcel stuck outside.

struct LockerBank::State {};

LockerBank::LockerBank(std::vector<Compartment> /*compartments*/, int /*holdMinutes*/)
    : state_(std::make_unique<State>()) {}

LockerBank::~LockerBank() = default;
LockerBank::LockerBank(LockerBank&&) noexcept = default;
LockerBank& LockerBank::operator=(LockerBank&&) noexcept = default;

DepositResult LockerBank::deposit(int /*time*/, const std::string& /*parcelId*/, Size /*size*/) {
  return {DepositStatus::NoSpace, ""};
}

bool LockerBank::pickup(int /*time*/, const std::string& /*parcelId*/) {
  return false;
}

std::optional<std::string> LockerBank::locate(const std::string& /*parcelId*/) const {
  return std::nullopt;
}

int LockerBank::freeCount(Size /*size*/) const {
  return 0;
}

std::vector<std::string> LockerBank::longestWaiting(int /*k*/) const {
  return {};
}

void LockerBank::advance(int /*time*/) {}

std::vector<std::string> LockerBank::returned() const {
  return {};
}

std::vector<std::string> LockerBank::waiting() const {
  return {};
}

}  // namespace lockerhub
