#include "io/Loaders.hpp"

#include <stdexcept>

namespace vanroute {

namespace {

void expectFields(const CsvRow& row, std::size_t count) {
  if (row.fields.size() != count) {
    throw std::runtime_error("line " + std::to_string(row.line) + ": expected " + std::to_string(count) +
                             " fields, got " + std::to_string(row.fields.size()));
  }
}

int clockField(const CsvRow& row, std::size_t index) {
  try {
    return parseClock(row.fields.at(index));
  } catch (const std::exception& e) {
    throw std::runtime_error("line " + std::to_string(row.line) + ": " + e.what());
  }
}

}  // namespace

RoadGraph loadRoads(const std::vector<CsvRow>& rows) {
  RoadGraph graph;
  for (const CsvRow& row : rows) {
    expectFields(row, 3);
    graph.addRoad(row.fields[0], row.fields[1], toInt(row, 2));
  }
  return graph;
}

std::vector<Van> loadVans(const std::vector<CsvRow>& rows) {
  std::vector<Van> vans;
  for (const CsvRow& row : rows) {
    expectFields(row, 5);
    vans.emplace_back(row.fields[0], row.fields[1], toInt(row, 2), clockField(row, 3), clockField(row, 4));
  }
  return vans;
}

std::vector<Parcel> loadParcels(const std::vector<CsvRow>& rows) {
  std::vector<Parcel> parcels;
  for (const CsvRow& row : rows) {
    expectFields(row, 5);
    const std::string& status = row.fields[4];
    if (status != "ready" && status != "cancelled") {
      throw std::runtime_error("line " + std::to_string(row.line) + ": unknown status \"" + status + "\"");
    }
    Parcel p;
    p.id = row.fields[0];
    p.destination = row.fields[1];
    p.size = toInt(row, 2);
    p.deadline = clockField(row, 3);
    p.cancelled = status == "cancelled";
    parcels.push_back(p);
  }
  return parcels;
}

}  // namespace vanroute
