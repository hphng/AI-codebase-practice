#pragma once

#include <string>
#include <vector>

namespace shiftplan {

using CsvRow = std::vector<std::string>;

// Splits CSV text into rows of fields. Fields are separated by commas and trimmed of surrounding
// spaces. Blank lines and lines starting with '#' are skipped. No quoting: the station files never
// contain commas inside a field.
std::vector<CsvRow> parseCsv(const std::string& text);

// Reads a CSV file and drops its header row. Throws std::runtime_error if the file can't be opened.
std::vector<CsvRow> readCsv(const std::string& path);

}  // namespace shiftplan
