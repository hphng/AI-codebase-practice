#include "util/Strings.hpp"

#include <algorithm>
#include <cctype>

namespace returns {

std::string toLower(std::string text) {
  std::transform(text.begin(), text.end(), text.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return text;
}

std::string join(const std::vector<std::string>& parts, const std::string& separator) {
  std::string out;
  for (std::size_t i = 0; i < parts.size(); ++i) {
    if (i > 0) out += separator;
    out += parts[i];
  }
  return out;
}

}  // namespace returns
