// shiftplan-cli [DATA_DIR]
// Plans the shift described in DATA_DIR (shift.csv, crews.csv, jobs.csv, trucks.csv) and prints the
// shift report. DATA_DIR defaults to "data".

#include <exception>
#include <iostream>

#include "report/Report.hpp"

int main(int argc, char** argv) {
  const std::string dataDir = argc > 1 ? argv[1] : "data";
  try {
    std::cout << shiftplan::runReport(dataDir);
  } catch (const std::exception& e) {
    std::cerr << "shiftplan-cli: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
