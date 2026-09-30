#include <fstream>
#include <sstream>
#include <string>

#include "report/Report.hpp"
#include "test_framework.hpp"

using namespace shiftplan;

TEST(Report, SampleShiftMatchesTheExpectedReport) {
  std::ifstream file(std::string(SAMPLE_DATA_DIR) + "/expected_report.txt", std::ios::binary);
  ASSERT_TRUE(file.good());
  std::ostringstream buffer;
  buffer << file.rdbuf();
  std::string expected;
  for (char c : buffer.str()) {
    if (c != '\r') expected += c;
  }
  EXPECT_EQ(runReport(SAMPLE_DATA_DIR), expected);
}
