#include <algorithm>
#include <fstream>
#include <sstream>

#include "TestData.hpp"
#include "report/Report.hpp"
#include "service/ReturnService.hpp"
#include "test_framework.hpp"

using namespace returns;
using namespace testdata;

namespace {

// C-1 (standard) bought on 2024-03-01:
//   CABLE  x3  books-ish accessory, no fee
//   COAT   x1  apparel
//   CLEAR  x1  apparel, final sale
// C-2 (standard) bought several shirts for the velocity scenarios.
// O-GHOST belongs to a customer that no longer exists.
struct World {
  ReturnService service;

  World() : service(standardConfig(), makeOrders(), makeCustomers()) {}

  static OrderRepository makeOrders() {
    OrderRepository orders;
    orders.add(order("O-1", "C-1", "2024-03-01",
                     {item("CABLE", "accessories", 1299, 3), item("COAT", "apparel", 12000, 1),
                      item("CLEAR", "apparel", 3000, 1, /*finalSale=*/true)}));
    orders.add(order("O-2", "C-2", "2024-02-20",
                     {item("S1", "apparel", 1500, 1), item("S2", "apparel", 1500, 1), item("S3", "apparel", 1500, 1),
                      item("S4", "apparel", 1500, 1), item("S5", "apparel", 1500, 1)}));
    orders.add(order("O-GHOST", "C-404", "2024-03-01", {item("X", "apparel", 1000, 1)}));
    orders.add(order("O-3", "C-3", "2024-02-20", {item("S6", "apparel", 1500, 1)}));
    return orders;
  }

  // C-3 is a second account in C-2's household (same address, different spelling).
  static CustomerRepository makeCustomers() {
    CustomerRepository customers;
    customers.add(customer("C-1"));
    Customer c2 = customer("C-2");
    c2.addresses = {"55 Harbor View"};
    customers.add(c2);
    Customer c3 = customer("C-3");
    c3.addresses = {"55 HARBOR VIEW "};
    customers.add(c3);
    return customers;
  }
};

}  // namespace

TEST(Service, ApprovesValidReturn) {
  World w;
  const Decision d = w.service.submit(request("R-1", "O-1", "COAT", 1, "2024-03-05"));
  ASSERT_TRUE(d.approved);
  EXPECT_EQ(d.refund, 12000);
  EXPECT_EQ(w.service.orders().find("O-1")->findItem("COAT")->returnedQuantity, 1);
}

TEST(Service, DeniesUnknownOrderSkuAndCustomer) {
  World w;
  EXPECT_FALSE(w.service.submit(request("R-1", "O-404", "COAT", 1, "2024-03-05")).approved);
  EXPECT_FALSE(w.service.submit(request("R-2", "O-1", "NOPE", 1, "2024-03-05")).approved);
  const Decision ghost = w.service.submit(request("R-3", "O-GHOST", "X", 1, "2024-03-05"));
  EXPECT_FALSE(ghost.approved);
  ASSERT_EQ(ghost.reasons.size(), static_cast<std::size_t>(1));
  EXPECT_CONTAINS(ghost.reasons[0], "unknown customer");
}

TEST(Service, CannotReturnTheSameUnitTwice) {
  World w;
  EXPECT_TRUE(w.service.submit(request("R-1", "O-1", "COAT", 1, "2024-03-05")).approved);
  EXPECT_FALSE(w.service.submit(request("R-2", "O-1", "COAT", 1, "2024-03-06")).approved);
}

TEST(Service, PartialReturnsUpToPurchasedQuantity) {
  World w;
  EXPECT_TRUE(w.service.submit(request("R-1", "O-1", "CABLE", 2, "2024-03-05")).approved);
  EXPECT_FALSE(w.service.submit(request("R-2", "O-1", "CABLE", 2, "2024-03-06")).approved);
  EXPECT_TRUE(w.service.submit(request("R-3", "O-1", "CABLE", 1, "2024-03-07")).approved);
  EXPECT_EQ(w.service.orders().find("O-1")->findItem("CABLE")->returnableQuantity(), 0);
}

TEST(Service, FinalSaleItemsAreNotRefunded) {
  World w;
  const Decision d = w.service.submit(request("R-1", "O-1", "CLEAR", 1, "2024-03-05", "changed_mind"));
  EXPECT_FALSE(d.approved);
  EXPECT_EQ(d.refund, 0);
}

TEST(Service, VelocityLimitWithChronologicalBatch) {
  World w;  // limit: 3 approved returns per 10 days
  EXPECT_TRUE(w.service.submit(request("R-1", "O-2", "S1", 1, "2024-03-01", "defective")).approved);
  EXPECT_TRUE(w.service.submit(request("R-2", "O-2", "S2", 1, "2024-03-02", "defective")).approved);
  EXPECT_TRUE(w.service.submit(request("R-3", "O-2", "S3", 1, "2024-03-03", "defective")).approved);
  EXPECT_FALSE(w.service.submit(request("R-4", "O-2", "S4", 1, "2024-03-04", "defective")).approved);
}

TEST(Service, VelocityLimitWithReplayedBatch) {
  World w;
  EXPECT_TRUE(w.service.submit(request("R-1", "O-2", "S1", 1, "2024-03-10", "defective")).approved);
  EXPECT_TRUE(w.service.submit(request("R-2", "O-2", "S2", 1, "2024-03-11", "defective")).approved);
  EXPECT_TRUE(w.service.submit(request("R-3", "O-2", "S3", 1, "2024-02-25", "defective")).approved);
  EXPECT_TRUE(w.service.submit(request("R-4", "O-2", "S4", 1, "2024-03-12", "defective")).approved);
  EXPECT_FALSE(w.service.submit(request("R-5", "O-2", "S5", 1, "2024-03-15", "defective")).approved);
}

TEST(Service, DeniedReturnsDontCountTowardsTheLimit) {
  World w;  // limit: 3 approved returns per 10 days
  EXPECT_FALSE(w.service.submit(request("R-1", "O-1", "CABLE", 9, "2024-03-02")).approved);  // more than bought
  EXPECT_FALSE(w.service.submit(request("R-2", "O-1", "CABLE", 9, "2024-03-03")).approved);
  EXPECT_FALSE(w.service.submit(request("R-3", "O-1", "CABLE", 9, "2024-03-04")).approved);
  const Decision d = w.service.submit(request("R-4", "O-1", "COAT", 1, "2024-03-05"));
  EXPECT_TRUE(d.approved);
}

TEST(Service, VelocityLimitAppliesToTheWholeHousehold) {
  World w;
  EXPECT_TRUE(w.service.submit(request("R-1", "O-2", "S1", 1, "2024-03-01", "defective")).approved);
  EXPECT_TRUE(w.service.submit(request("R-2", "O-2", "S2", 1, "2024-03-02", "defective")).approved);
  EXPECT_TRUE(w.service.submit(request("R-3", "O-2", "S3", 1, "2024-03-03", "defective")).approved);
  const Decision d = w.service.submit(request("R-4", "O-3", "S6", 1, "2024-03-04", "defective"));
  EXPECT_FALSE(d.approved);
  ASSERT_EQ(d.reasons.size(), static_cast<std::size_t>(1));
  EXPECT_CONTAINS(d.reasons[0], "velocity");
}

TEST(Service, WritesAnAuditEntryForEveryRequest) {
  World w;
  w.service.submit(request("R-1", "O-1", "COAT", 1, "2024-03-05"));
  w.service.submit(request("R-2", "O-404", "COAT", 1, "2024-03-05"));
  ASSERT_EQ(w.service.auditLog().size(), static_cast<std::size_t>(2));
  EXPECT_EQ(w.service.auditLog()[0].request.id, std::string("R-1"));
  EXPECT_TRUE(w.service.auditLog()[0].decision.approved);
  EXPECT_EQ(w.service.auditLog()[1].request.orderId, std::string("O-404"));
}

TEST(Service, NotifiesTheCustomerOfEachDecision) {
  World w;
  w.service.submit(request("R-1", "O-1", "COAT", 1, "2024-03-05"));
  w.service.submit(request("R-2", "O-1", "CABLE", 5, "2024-03-05"));  // more than purchased

  ASSERT_EQ(w.service.outbox().size(), static_cast<std::size_t>(2));
  EXPECT_EQ(w.service.outbox()[0].customerId, std::string("C-1"));
  EXPECT_EQ(w.service.outbox()[0].requestId, std::string("R-1"));
  EXPECT_CONTAINS(w.service.outbox()[0].message, "approved");
  EXPECT_CONTAINS(w.service.outbox()[1].message, "declined");
}

// ---------- End to end: data/ directory vs. data/expected_report.txt ----------

TEST(SampleData, AllDataFilesParse) {
  for (const char* file : {"policies.json", "customers.json", "orders.json", "requests.json"}) {
    EXPECT_NO_THROW(json::parseFile(std::string(SAMPLE_DATA_DIR) + "/" + file));
  }
}

TEST(SampleData, ReportMatchesExpectedOutput) {
  std::ifstream in(std::string(SAMPLE_DATA_DIR) + "/expected_report.txt", std::ios::binary);
  ASSERT_TRUE(in.good());
  std::ostringstream buffer;
  buffer << in.rdbuf();

  std::string expected = buffer.str();  // compare ignoring CRLF vs LF
  expected.erase(std::remove(expected.begin(), expected.end(), '\r'), expected.end());

  std::string actual;
  EXPECT_NO_THROW(actual = runReport(SAMPLE_DATA_DIR));
  EXPECT_EQ(actual, expected);
}
