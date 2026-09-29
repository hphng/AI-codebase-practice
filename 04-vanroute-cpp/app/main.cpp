// vanroute-cli: plans today's deliveries from a data directory and prints the dispatch report.
//
//   vanroute-cli [DATA_DIR]      (default: ./data)

#include <exception>
#include <iostream>
#include <string>

#include "report/Report.hpp"

int main(int argc, char** argv) {
  const std::string dataDir = argc > 1 ? argv[1] : "data";
  try {
    std::cout << vanroute::runReport(dataDir);
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
}
