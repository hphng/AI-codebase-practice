#include "io/Csv.hpp"

#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace vanroute {

namespace {

std::string trim(const std::string& text) {
  const char* ws = " \t\r\n";
  const auto begin = text.find_first_not_of(ws);
  if (begin == std::string::npos) return "";
  const auto end = text.find_last_not_of(ws);
  return text.substr(begin, end - begin + 1);
}

std::string stripComment(const std::string& line) {
  const auto hash = line.find('#');
  if (hash != std::string::npos) return line.substr(0, hash);
  return line;
}

}  // namespace

std::vector<CsvRow> parseCsv(const std::string& text) {
  std::vector<CsvRow> rows;
  std::istringstream in(text);
  std::string line;
  int number = 0;

  while (std::getline(in, line)) {
    ++number;
    const std::string content = trim(stripComment(line));
    if (content.empty()) continue;

    CsvRow row;
    row.line = number;
    std::istringstream fields(content);
    std::string field;
    while (std::getline(fields, field, ',')) row.fields.push_back(trim(field));
    if (content.back() == ',') row.fields.push_back("");  // trailing empty field
    rows.push_back(std::move(row));
  }
  return rows;
}

std::vector<CsvRow> readCsv(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("cannot open " + path);
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return parseCsv(buffer.str());
}

int toInt(const CsvRow& row, std::size_t index) {
  if (index >= row.fields.size()) {
    throw std::runtime_error("line " + std::to_string(row.line) + ": missing field " + std::to_string(index + 1));
  }
  const std::string& text = row.fields[index];
  std::size_t used = 0;
  int value = 0;
  try {
    value = std::stoi(text, &used);
  } catch (const std::exception&) {
    used = 0;
  }
  if (text.empty() || used != text.size()) {
    throw std::runtime_error("line " + std::to_string(row.line) + ": \"" + text + "\" is not an integer");
  }
  return value;
}

int parseClock(const std::string& text) {
  if (text.size() != 5 || text[2] != ':' || !std::isdigit(static_cast<unsigned char>(text[0])) ||
      !std::isdigit(static_cast<unsigned char>(text[1])) || !std::isdigit(static_cast<unsigned char>(text[3])) ||
      !std::isdigit(static_cast<unsigned char>(text[4]))) {
    throw std::runtime_error("expected HH:MM, got \"" + text + "\"");
  }
  const int hours = std::stoi(text.substr(0, 2));
  const int minutes = std::stoi(text.substr(3, 2));
  if (hours > 23 || minutes > 59) throw std::runtime_error("no such time \"" + text + "\"");
  return hours * 60 + minutes;
}

std::string formatClock(int minutes) {
  std::ostringstream out;
  out << std::setfill('0') << std::setw(2) << minutes / 60 << ':' << std::setw(2) << minutes % 60;
  return out.str();
}

}  // namespace vanroute
