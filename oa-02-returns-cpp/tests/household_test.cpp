#include <chrono>
#include <string>
#include <vector>

#include "fraud/Households.hpp"
#include "test_framework.hpp"

using namespace returns;

namespace {

Customer person(std::string id, std::vector<std::string> addresses, std::vector<std::string> cards = {}) {
  Customer c;
  c.id = std::move(id);
  c.name = c.id;
  c.addresses = std::move(addresses);
  c.paymentFingerprints = std::move(cards);
  return c;
}

}  // namespace

TEST(Households, EveryoneIsTheirOwnHouseholdWhenNothingIsShared) {
  const auto h = buildHouseholds({person("A", {"1 Elm St"}), person("B", {"2 Elm St"}, {"card-b"})});
  ASSERT_EQ(h.size(), static_cast<std::size_t>(2));
  EXPECT_EQ(h.at("A"), std::string("A"));
  EXPECT_EQ(h.at("B"), std::string("B"));
}

TEST(Households, SharedAddressJoinsAHousehold) {
  const auto h = buildHouseholds({person("C2", {"9 Pine Rd"}), person("C1", {"9 Pine Rd"}), person("C3", {"3 Oak"})});
  EXPECT_EQ(h.at("C1"), std::string("C1"));
  EXPECT_EQ(h.at("C2"), std::string("C1"));
  EXPECT_EQ(h.at("C3"), std::string("C3"));
}

TEST(Households, SharedPaymentFingerprintJoinsAHousehold) {
  const auto h = buildHouseholds({person("X", {}, {"fp-1", "fp-2"}), person("Y", {}, {"fp-2"})});
  EXPECT_EQ(h.at("Y"), std::string("X"));
}

TEST(Households, LinksAreTransitive) {
  // A-B share a card, B-C share an address, C-D share a card. E is alone.
  const auto h = buildHouseholds({
      person("D", {"4 Main"}, {"card-cd"}),
      person("B", {"7 Birch Ln"}, {"card-ab"}),
      person("C", {"7 Birch Ln"}, {"card-cd"}),
      person("A", {"1 Main"}, {"card-ab"}),
      person("E", {"5 Main"}, {"card-e"}),
  });
  for (const char* id : {"A", "B", "C", "D"}) EXPECT_EQ(h.at(id), std::string("A"));
  EXPECT_EQ(h.at("E"), std::string("E"));
}

TEST(Households, AddressesMatchIgnoringCaseAndSurroundingSpaces) {
  const auto h = buildHouseholds({person("P", {"  12 Oak St "}), person("Q", {"12 OAK st"})});
  EXPECT_EQ(h.at("Q"), std::string("P"));
}

TEST(Households, PaymentFingerprintsAreCaseSensitive) {
  const auto h = buildHouseholds({person("P", {}, {"AbC123"}), person("Q", {}, {"abc123"})});
  EXPECT_EQ(h.at("Q"), std::string("Q"));
}

TEST(Households, BlankValuesNeverLink) {
  const auto h = buildHouseholds({person("P", {"", "   "}, {""}), person("Q", {""}, {""})});
  EXPECT_EQ(h.at("P"), std::string("P"));
  EXPECT_EQ(h.at("Q"), std::string("Q"));
}

TEST(Households, AddressNeverMatchesAPaymentFingerprint) {
  const auto h = buildHouseholds({person("P", {"x1"}), person("Q", {}, {"x1"})});
  EXPECT_EQ(h.at("Q"), std::string("Q"));
}

TEST(Households, RepresentativeIsTheLexicographicallySmallestId) {
  const auto h = buildHouseholds({person("C9", {"h"}), person("C100", {"h"}), person("C10", {"h"})});
  for (const char* id : {"C9", "C100", "C10"}) EXPECT_EQ(h.at(id), std::string("C10"));
}

TEST(Households, ScalesToLargeCustomerBases) {
  // 200,000 customers in 400 households of 500. Within a household, customer i shares an address
  // with i+1 and a card with i+2, so every household is a long chain.
  const int households = 400;
  const int size = 500;
  std::vector<Customer> customers;
  customers.reserve(households * size);
  for (int h = 0; h < households; ++h) {
    for (int i = 0; i < size; ++i) {
      const std::string prefix = "H" + std::to_string(h) + "-";
      customers.push_back(person(prefix + "C" + std::to_string(100000 + i),
                                 {prefix + "addr" + std::to_string(i / 2), prefix + "addr" + std::to_string((i + 1) / 2 + 100000)},
                                 {prefix + "card" + std::to_string(i / 3)}));
    }
  }

  const auto started = std::chrono::steady_clock::now();
  const auto result = buildHouseholds(customers);
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count();

  ASSERT_EQ(result.size(), customers.size());
  EXPECT_EQ(result.at("H7-C100499"), std::string("H7-C100000"));
  EXPECT_EQ(result.at("H399-C100250"), std::string("H399-C100000"));
  EXPECT_TRUE(ms < 3000);
}
