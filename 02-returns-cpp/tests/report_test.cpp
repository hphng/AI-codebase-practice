#include <vector>

#include "report/Report.hpp"
#include "test_framework.hpp"

using namespace returns;

namespace {

Decision approved(Cents refund, Cents fee) {
  Decision d;
  d.approved = true;
  d.refund = refund;
  d.restockingFee = fee;
  return d;
}

}  // namespace

TEST(Report, TotalRefundedIsWhatCustomersGetBack) {
  const std::vector<Decision> decisions{approved(1699, 300), approved(12000, 0), Decision::deny("R-3", "nope")};
  EXPECT_EQ(totalRefunded(decisions), 13699);
}

TEST(Report, TotalRefundedOfNothing) {
  EXPECT_EQ(totalRefunded({}), 0);
  EXPECT_EQ(totalRefunded({Decision::deny("R-1", "x"), Decision::deny("R-2", "y")}), 0);
}
