#include "service/ReturnService.hpp"

#include "util/Strings.hpp"

namespace returns {

ReturnService::ReturnService(PolicyConfig config, OrderRepository orders, CustomerRepository customers)
    : config_(std::move(config)),
      orders_(std::move(orders)),
      customers_(std::move(customers)),
      velocity_(config_.velocityWindowDays()),
      engine_(makeDefaultRules(config_)) {}

Decision ReturnService::submit(ReturnRequest request) {
  Decision decision = decide(request);
  if (decision.approved) applyReturn(request);

  audit_.push_back(AuditEntry{std::move(request), decision});
  notifyCustomer(request, decision);
  return decision;
}

Decision ReturnService::decide(const ReturnRequest& request) const {
  const Order* order = orders_.find(request.orderId);
  if (!order) return Decision::deny(request.id, "unknown order " + request.orderId);

  const LineItem* item = order->findItem(request.sku);
  if (!item) return Decision::deny(request.id, "SKU " + request.sku + " is not on order " + order->id);

  const Customer* customer = customers_.find(order->customerId);
  if (!customer) return Decision::deny(request.id, "unknown customer " + order->customerId);

  const EvalContext ctx{*order, *item, *customer, request, config_.forCategory(item->category), config_,
                        velocity_};
  return engine_.evaluate(ctx);
}

void ReturnService::applyReturn(const ReturnRequest& request) {
  orders_.recordReturn(request.orderId, request.sku, request.quantity);
  if (const Order* order = orders_.find(request.orderId)) {
    velocity_.record(order->customerId, request.requestDate);
  }
}

void ReturnService::notifyCustomer(const ReturnRequest& request, const Decision& decision) {
  const Order* order = orders_.find(request.orderId);
  if (!order) return;  // no order, nobody to notify

  std::string message =
      decision.approved
          ? "Return " + request.id + " approved. Refund: " + formatCents(decision.refund)
          : "Return " + request.id + " declined: " + join(decision.reasons, "; ");
  outbox_.push_back(Notification{order->customerId, request.id, std::move(message)});
}

}  // namespace returns
