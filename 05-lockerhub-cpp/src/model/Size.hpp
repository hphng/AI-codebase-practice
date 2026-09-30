#pragma once

#include <string>

namespace lockerhub {

// Size class of a compartment or a parcel, smallest first.
enum class Size { Small = 0, Medium = 1, Large = 2 };

constexpr int kSizeCount = 3;

// "S", "M" or "L" (the codes used in the data files). Throws std::invalid_argument otherwise.
Size parseSize(const std::string& code);

// 'S', 'M' or 'L'.
char sizeCode(Size size);

// A parcel fits a compartment of its own size class or any larger one.
bool fits(Size parcel, Size compartment);

}  // namespace lockerhub
