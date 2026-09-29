#pragma once

#include <string>
#include <vector>

namespace returns {

// ASCII lower-casing (category names, tiers and reasons are ASCII identifiers).
std::string toLower(std::string text);

std::string join(const std::vector<std::string>& parts, const std::string& separator);

}  // namespace returns
