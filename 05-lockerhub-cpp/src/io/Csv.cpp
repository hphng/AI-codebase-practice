#include "io/Csv.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace lockerhub {

namespace {

std::string trim(const std::string& s) {
  const auto first = s.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  const auto last = s.find_last_not_of(" \t\r\n");
  return s.substr(first, last - first + 1);
}

}  // namespace

std::vector<CsvRow> parseCsv(const std::string& text) {
  std::vector<CsvRow> rows;
  std::istringstream in(text);
  std::string line;
  while (std::getline(in, line)) {
    const std::string content = trim(line);
    if (content.empty() || content[0] == '#') continue;

    CsvRow row;
    std::size_t start = 0;
    while (true) {
      const auto comma = content.find(',', start);
      if (comma == std::string::npos) {
        row.push_back(trim(content.substr(start)));
        break;
      }
      row.push_back(trim(content.substr(start, comma - start)));
      start = comma + 1;
    }
    rows.push_back(std::move(row));
  }
  return rows;
}

std::vector<CsvRow> readCsv(const std::string& path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) throw std::runtime_error("cannot open " + path);
  std::ostringstream buffer;
  buffer << file.rdbuf();
  auto rows = parseCsv(buffer.str());
  if (!rows.empty()) rows.erase(rows.begin());  // header
  return rows;
}

}  // namespace lockerhub
