// lockerhub-cli [DATA_DIR]
// Replays DATA_DIR/events.csv through the station's lockers and prints the station report.
// DATA_DIR defaults to "data".

#include <exception>
#include <iostream>

#include "report/Report.hpp"

int main(int argc, char** argv) {
  const std::string dataDir = argc > 1 ? argv[1] : "data";
  try {
    std::cout << lockerhub::runReport(dataDir);
  } catch (const std::exception& e) {
    std::cerr << "lockerhub-cli: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
