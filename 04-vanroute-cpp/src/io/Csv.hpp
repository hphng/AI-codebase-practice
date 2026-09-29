#pragma once

#include <string>
#include <vector>

namespace vanroute {

struct CsvRow {
  int line = 0;                     // 1-based line number in the source
  std::vector<std::string> fields;  // trimmed
};

// Parses comma-separated text. Blank lines are skipped, and '#' starts a comment that runs to the
// end of the line (a line that is only a comment is skipped). Fields are trimmed of spaces, tabs
// and '\r'. Quoting is not supported.
std::vector<CsvRow> parseCsv(const std::string& text);

// Reads a file and parses it with parseCsv. Throws std::runtime_error if it can't be opened.
std::vector<CsvRow> readCsv(const std::string& path);

// Field helpers. They throw std::runtime_error naming the line on bad input.
int toInt(const CsvRow& row, std::size_t index);
// "HH:MM" (24h) -> minutes after midnight, e.g. "08:30" -> 510.
int parseClock(const std::string& text);
std::string formatClock(int minutes);  // 510 -> "08:30"

}  // namespace vanroute
