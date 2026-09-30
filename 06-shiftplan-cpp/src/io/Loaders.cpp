#include "io/Loaders.hpp"

#include <stdexcept>
#include <unordered_set>

#include "io/Clock.hpp"
#include "io/Csv.hpp"

namespace shiftplan {

namespace {

[[noreturn]] void fail(const std::string& source, std::size_t row, const std::string& message) {
  throw std::runtime_error(source + " row " + std::to_string(row + 1) + ": " + message);
}

void expectColumns(const std::vector<std::string>& row, std::size_t count, const std::string& source,
                   std::size_t index) {
  if (row.size() != count) {
    fail(source, index, "expected " + std::to_string(count) + " columns, got " + std::to_string(row.size()));
  }
}

int toInt(const std::string& text, const std::string& source, std::size_t index) {
  std::size_t used = 0;
  int value = 0;
  try {
    value = std::stoi(text, &used);
  } catch (const std::exception&) {
    fail(source, index, "'" + text + "' is not a number");
  }
  if (used != text.size()) fail(source, index, "'" + text + "' is not a whole number");
  return value;
}

int toClock(const std::string& text, const std::string& source, std::size_t index) {
  try {
    return parseClock(text);
  } catch (const std::invalid_argument& e) {
    fail(source, index, e.what());
  }
}

std::vector<std::string> splitIds(const std::string& text) {
  std::vector<std::string> ids;
  std::string current;
  for (char c : text + ";") {
    if (c == ';') {
      if (!current.empty()) ids.push_back(current);
      current.clear();
    } else if (c != ' ') {
      current += c;
    }
  }
  return ids;
}

}  // namespace

Shift shiftFromRows(const Rows& rows, const std::string& source) {
  if (rows.size() != 1) throw std::runtime_error(source + ": expected exactly one shift row");
  expectColumns(rows[0], 3, source, 0);
  Shift shift{rows[0][0], toClock(rows[0][1], source, 0), toInt(rows[0][2], source, 0)};
  if (shift.minutes <= 0) fail(source, 0, "shift length must be positive");
  return shift;
}

CrewPool crewsFromRows(const Rows& rows, const std::string& source) {
  CrewPool crews;
  for (std::size_t i = 0; i < rows.size(); ++i) {
    expectColumns(rows[i], 2, source, i);
    const int count = toInt(rows[i][1], source, i);
    if (rows[i][0].empty()) fail(source, i, "crew type is empty");
    if (count <= 0) fail(source, i, "crew count must be positive");
    if (!crews.emplace(rows[i][0], count).second) fail(source, i, "crew type " + rows[i][0] + " listed twice");
  }
  return crews;
}

std::vector<Job> jobsFromRows(const Rows& rows, const std::string& source) {
  std::vector<Job> jobs;
  for (std::size_t i = 0; i < rows.size(); ++i) {
    const auto& row = rows[i];
    expectColumns(row, 5, source, i);
    Job job;
    job.id = row[0];
    job.minutes = toInt(row[1], source, i);
    job.priority = toInt(row[2], source, i);
    job.crew = row[3];
    job.after = splitIds(row[4]);
    jobs.push_back(job);
  }
  return jobs;
}

std::vector<Truck> trucksFromRows(const Rows& rows, const std::string& source) {
  std::vector<Truck> trucks;
  for (std::size_t i = 0; i < rows.size(); ++i) {
    expectColumns(rows[i], 3, source, i);
    trucks.push_back(Truck{rows[i][0], rows[i][1], toClock(rows[i][2], source, i)});
  }
  return trucks;
}

SiteData loadSite(const std::string& dir) {
  SiteData site;
  site.shift = shiftFromRows(readCsv(dir + "/shift.csv"), "shift.csv");
  site.crews = crewsFromRows(readCsv(dir + "/crews.csv"), "crews.csv");
  site.jobs = jobsFromRows(readCsv(dir + "/jobs.csv"), "jobs.csv");
  site.trucks = trucksFromRows(readCsv(dir + "/trucks.csv"), "trucks.csv");

  std::unordered_set<std::string> ids;
  for (const Job& job : site.jobs) ids.insert(job.id);
  for (const Truck& truck : site.trucks) {
    if (!ids.count(truck.loadJob)) {
      throw std::runtime_error("trucks.csv: truck " + truck.id + " is loaded by unknown job " + truck.loadJob);
    }
  }
  return site;
}

}  // namespace shiftplan
