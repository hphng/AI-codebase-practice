#include "io/Loaders.hpp"

#include <stdexcept>
#include <unordered_set>

#include "io/Clock.hpp"
#include "io/Csv.hpp"

namespace lockerhub {

namespace {

[[noreturn]] void fail(const std::string& source, std::size_t row, const std::string& message) {
  throw std::runtime_error(source + " row " + std::to_string(row + 1) + ": " + message);
}

void expectColumns(const CsvRow& row, std::size_t count, const std::string& source, std::size_t index) {
  if (row.size() != count) {
    fail(source, index, "expected " + std::to_string(count) + " columns, got " + std::to_string(row.size()));
  }
}

int toPositiveInt(const std::string& text, const std::string& source, std::size_t index) {
  std::size_t used = 0;
  int value = 0;
  try {
    value = std::stoi(text, &used);
  } catch (const std::exception&) {
    fail(source, index, "'" + text + "' is not a number");
  }
  if (used != text.size() || value <= 0) fail(source, index, "'" + text + "' must be a positive whole number");
  return value;
}

}  // namespace

StationConfig stationFromRows(const std::vector<CsvRow>& rows, const std::string& source) {
  if (rows.size() != 1) throw std::runtime_error(source + ": expected exactly one station row");
  const CsvRow& row = rows[0];
  expectColumns(row, 3, source, 0);
  if (row[0].empty()) fail(source, 0, "station id is empty");
  return StationConfig{row[0], row[1], toPositiveInt(row[2], source, 0)};
}

std::vector<Compartment> compartmentsFromRows(const std::vector<CsvRow>& rows, const std::string& source) {
  std::vector<Compartment> compartments;
  std::unordered_set<std::string> seen;
  for (std::size_t i = 0; i < rows.size(); ++i) {
    const CsvRow& row = rows[i];
    expectColumns(row, 2, source, i);
    if (row[0].empty()) fail(source, i, "compartment id is empty");
    if (!seen.insert(row[0]).second) fail(source, i, "duplicate compartment " + row[0]);
    try {
      compartments.push_back(Compartment{row[0], parseSize(row[1])});
    } catch (const std::invalid_argument& e) {
      fail(source, i, e.what());
    }
  }
  return compartments;
}

std::vector<Event> eventsFromRows(const std::vector<CsvRow>& rows, const std::string& source) {
  std::vector<Event> events;
  for (std::size_t i = 0; i < rows.size(); ++i) {
    const CsvRow& row = rows[i];
    expectColumns(row, 5, source, i);

    Event event;
    try {
      event.minute = parseStamp(row[0], row[1]);
    } catch (const std::invalid_argument& e) {
      fail(source, i, e.what());
    }
    if (!events.empty() && event.minute < events.back().minute) fail(source, i, "event is out of time order");

    const std::string& kind = row[2];
    if (kind == "DEPOSIT") {
      event.kind = EventKind::Deposit;
      try {
        event.size = parseSize(row[4]);
      } catch (const std::invalid_argument& e) {
        fail(source, i, e.what());
      }
    } else if (kind == "PICKUP") {
      event.kind = EventKind::Pickup;
    } else if (kind == "CLOSE") {
      event.kind = EventKind::Close;
    } else {
      fail(source, i, "unknown event '" + kind + "'");
    }

    event.parcelId = row[3];
    if (event.kind != EventKind::Close && event.parcelId.empty()) fail(source, i, "parcel id is empty");
    if (event.kind != EventKind::Deposit && !row[4].empty()) fail(source, i, "size is only allowed on DEPOSIT");

    events.push_back(event);
  }
  return events;
}

StationConfig loadStation(const std::string& path) {
  return stationFromRows(readCsv(path), path);
}

std::vector<Compartment> loadCompartments(const std::string& path) {
  return compartmentsFromRows(readCsv(path), path);
}

std::vector<Event> loadEvents(const std::string& path) {
  return eventsFromRows(readCsv(path), path);
}

}  // namespace lockerhub
