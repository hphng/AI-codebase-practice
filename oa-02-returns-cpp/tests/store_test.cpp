#include "TestData.hpp"
#include "fraud/ReturnVelocityTracker.hpp"
#include "store/CustomerRepository.hpp"
#include "store/OrderRepository.hpp"
#include "test_framework.hpp"

using namespace returns;
using namespace testdata;

// ---------- OrderRepository ----------

TEST(Orders, FindsOrdersAndItems) {
  OrderRepository repo;
  repo.add(order("O-1", "C-1", "2024-01-01", {item("A", "books", 100, 1)}));
  ASSERT_TRUE(repo.find("O-1") != nullptr);
  EXPECT_TRUE(repo.find("O-1")->findItem("A") != nullptr);
  EXPECT_TRUE(repo.find("O-1")->findItem("Z") == nullptr);
  EXPECT_TRUE(repo.find("nope") == nullptr);
}

TEST(Orders, RejectsDuplicateIds) {
  OrderRepository repo;
  repo.add(order("O-1", "C-1", "2024-01-01", {}));
  EXPECT_THROW(repo.add(order("O-1", "C-2", "2024-01-02", {})), std::invalid_argument);
}

TEST(Orders, RecordReturnUpdatesReturnedQuantity) {
  OrderRepository repo;
  repo.add(order("O-1", "C-1", "2024-01-01", {item("A", "books", 100, 3), item("B", "books", 100, 1)}));

  EXPECT_TRUE(repo.recordReturn("O-1", "A", 2));
  EXPECT_EQ(repo.find("O-1")->findItem("A")->returnedQuantity, 2);
  EXPECT_EQ(repo.find("O-1")->findItem("A")->returnableQuantity(), 1);
  EXPECT_EQ(repo.find("O-1")->findItem("B")->returnedQuantity, 0);
}

TEST(Orders, RecordReturnReportsUnknownOrderOrSku) {
  OrderRepository repo;
  repo.add(order("O-1", "C-1", "2024-01-01", {item("A", "books", 100, 1)}));
  EXPECT_FALSE(repo.recordReturn("O-2", "A", 1));
  EXPECT_FALSE(repo.recordReturn("O-1", "Z", 1));
}

// ---------- CustomerRepository ----------

TEST(Customers, FindsCustomers) {
  CustomerRepository repo;
  repo.add(customer("C-1", Tier::Prime));
  ASSERT_TRUE(repo.find("C-1") != nullptr);
  EXPECT_TRUE(repo.find("C-1")->tier == Tier::Prime);
  EXPECT_TRUE(repo.find("C-2") == nullptr);
  EXPECT_EQ(repo.size(), static_cast<std::size_t>(1));
}

// ---------- ReturnVelocityTracker ----------

TEST(Velocity, CountsReturnsInsideWindow) {
  ReturnVelocityTracker t(30);
  t.record("C", Date::parse("2024-03-01"));
  t.record("C", Date::parse("2024-03-10"));
  t.record("C", Date::parse("2024-03-20"));
  EXPECT_EQ(t.countInWindow("C", Date::parse("2024-03-20")), 3);
  EXPECT_EQ(t.countInWindow("C", Date::parse("2024-03-31")), 2);  // window starts Mar 2
  EXPECT_EQ(t.countInWindow("C", Date::parse("2024-03-05")), 1);  // later returns don't count
}

TEST(Velocity, CustomersAreIndependent) {
  ReturnVelocityTracker t(30);
  t.record("A", Date::parse("2024-03-01"));
  EXPECT_EQ(t.countInWindow("B", Date::parse("2024-03-01")), 0);
  EXPECT_EQ(t.countInWindow("A", Date::parse("2024-03-01")), 1);
}

TEST(Velocity, HandlesOutOfOrderRecording) {
  ReturnVelocityTracker t(30);
  t.record("C", Date::parse("2024-04-10"));
  t.record("C", Date::parse("2024-04-11"));
  t.record("C", Date::parse("2024-01-05"));  // a late-arriving old return
  t.record("C", Date::parse("2024-04-12"));
  t.record("C", Date::parse("2024-02-01"));

  EXPECT_EQ(t.countInWindow("C", Date::parse("2024-04-15")), 3);
  EXPECT_EQ(t.countInWindow("C", Date::parse("2024-02-03")), 2);
  EXPECT_EQ(t.countInWindow("C", Date::parse("2024-01-05")), 1);
}
