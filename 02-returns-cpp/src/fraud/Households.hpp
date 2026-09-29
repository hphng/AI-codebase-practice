#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "model/Models.hpp"

namespace returns {

// PART A: implement buildHouseholds (src/fraud/Households.cpp).
//
// Return-abuse limits apply per *household*, not per account: people open several accounts to
// dodge the limit. Two customers are in the same household if they share an address or a payment
// fingerprint, directly or through a chain of other customers (A shares a card with B, and B shares
// an address with C, so A, B and C are one household).
//
// Returns a map of customerId -> householdId for EVERY input customer, where householdId is the
// lexicographically smallest customer id in that household (std::string operator<). A customer who
// shares nothing is a household of one.
//
// Matching rules:
//  - Addresses match case-insensitively, ignoring leading/trailing whitespace
//    ("12 Oak St " matches "12 oak st"). Nothing else is normalized.
//  - Payment fingerprints match exactly (case-sensitive).
//  - Empty or whitespace-only values never link anyone.
//  - An address and a payment fingerprint never match each other, even if the text is equal.
//
// Performance: up to 200,000 customers with several values each. The solution must be close to
// linear in the total number of addresses + fingerprints. There is a performance test.
std::unordered_map<std::string, std::string> buildHouseholds(const std::vector<Customer>& customers);

}  // namespace returns
