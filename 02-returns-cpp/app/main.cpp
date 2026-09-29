// returns-cli: processes the return requests in a data directory and prints a report.
//
//   returns-cli [DATA_DIR]      (default: ./data)

#include <exception>
#include <iostream>
#include <string>

#include "report/Report.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif
  const std::string dataDir = argc > 1 ? argv[1] : "data";
  try {
    std::cout << returns::runReport(dataDir);
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
}
