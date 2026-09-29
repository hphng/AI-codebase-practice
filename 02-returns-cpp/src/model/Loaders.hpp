#pragma once

// Converts parsed JSON into domain objects. See data/*.json for the formats.

#include <vector>

#include "json/Json.hpp"
#include "model/Models.hpp"

namespace returns {

Customer customerFromJson(const json::Value& value);
Order orderFromJson(const json::Value& value);
ReturnRequest requestFromJson(const json::Value& value);

std::vector<Customer> customersFromJson(const json::Value& array);
std::vector<Order> ordersFromJson(const json::Value& array);
std::vector<ReturnRequest> requestsFromJson(const json::Value& array);

}  // namespace returns
