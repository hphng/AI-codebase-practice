#include "report/Report.hpp"

#include <iomanip>
#include <sstream>

#include "json/Json.hpp"
#include "model/Loaders.hpp"
#include "service/ReturnService.hpp"
#include "util/Strings.hpp"

namespace returns {

Cents totalRefunded(const std::vector<Decision>& decisions) {
  Cents total = 0;
  for (const Decision& d : decisions) {
    if (!d.approved) continue;
    total += d.refund + d.restockingFee;  // full value of the returned goods
  }
  return total;
}

std::string runReport(const std::string& dataDir) {
  const auto path = [&dataDir](const char* file) { return dataDir + "/" + file; };

  PolicyConfig config = PolicyConfig::fromJson(json::parseFile(path("policies.json")));

  CustomerRepository customers;
  for (auto& c : customersFromJson(json::parseFile(path("customers.json")))) customers.add(std::move(c));

  OrderRepository orders;
  for (auto& o : ordersFromJson(json::parseFile(path("orders.json")))) orders.add(std::move(o));

  const std::vector<ReturnRequest> requests = requestsFromJson(json::parseFile(path("requests.json")));

  ReturnService service(std::move(config), std::move(orders), std::move(customers));

  std::ostringstream out;
  out << std::left << std::setw(7) << "REQ" << std::setw(8) << "ORDER" << std::setw(28) << "ITEM"
      << std::setw(10) << "DECISION" << std::setw(10) << "REFUND" << std::setw(9) << "FEE"
      << "NOTES\n";

  int approved = 0;
  std::vector<Decision> decisions;
  for (const ReturnRequest& request : requests) {
    std::string itemName = request.sku;
    if (const Order* order = service.orders().find(request.orderId)) {
      if (const LineItem* item = order->findItem(request.sku)) itemName = item->name;
    }

    const Decision d = service.submit(request);
    decisions.push_back(d);
    if (d.approved) ++approved;

    out << std::setw(7) << request.id << std::setw(8) << request.orderId << std::setw(28) << itemName
        << std::setw(10) << (d.approved ? "APPROVED" : "DENIED")
        << std::setw(10) << (d.approved ? formatCents(d.refund) : "-")
        << std::setw(9) << (d.approved ? formatCents(d.restockingFee) : "-")
        << join(d.reasons, "; ") << "\n";
  }

  out << "\nNotifications (" << service.outbox().size() << "):\n";
  for (const Notification& n : service.outbox()) {
    out << "  to " << std::setw(6) << n.customerId << n.message << "\n";
  }

  out << "\nSummary: " << approved << " approved, " << requests.size() - approved << " denied, "
      << formatCents(totalRefunded(decisions)) << " refunded\n";
  return out.str();
}

}  // namespace returns
