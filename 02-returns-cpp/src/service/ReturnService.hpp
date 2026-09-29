#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "engine/ReturnEngine.hpp"
#include "fraud/Households.hpp"
#include "fraud/ReturnVelocityTracker.hpp"
#include "policy/PolicyConfig.hpp"
#include "store/CustomerRepository.hpp"
#include "store/OrderRepository.hpp"

namespace returns {

struct AuditEntry {
  ReturnRequest request;
  Decision decision;
};

struct Notification {
  std::string customerId;
  std::string requestId;
  std::string message;
};

// Entry point for processing return requests:
//   1. decide    - look up order/item/customer and run the rule engine
//   2. apply     - for approved returns, update the order and the household's velocity count
//   3. audit     - every request is appended to the audit log
//   4. notify    - the order's customer gets a message with the decision
class ReturnService {
 public:
  ReturnService(PolicyConfig config, OrderRepository orders, CustomerRepository customers);

  ReturnService(const ReturnService&) = delete;
  ReturnService& operator=(const ReturnService&) = delete;

  Decision submit(ReturnRequest request);

  const std::vector<AuditEntry>& auditLog() const { return audit_; }
  const std::vector<Notification>& outbox() const { return outbox_; }
  const OrderRepository& orders() const { return orders_; }

 private:
  Decision decide(const ReturnRequest& request) const;
  void applyReturn(const ReturnRequest& request);
  void notifyCustomer(const ReturnRequest& request, const Decision& decision);
  const std::string& householdOf(const std::string& customerId) const;

  PolicyConfig config_;
  OrderRepository orders_;
  CustomerRepository customers_;
  std::unordered_map<std::string, std::string> households_;  // customerId -> householdId
  ReturnVelocityTracker velocity_;
  ReturnEngine engine_;
  std::vector<AuditEntry> audit_;
  std::vector<Notification> outbox_;
};

}  // namespace returns
