#include "billing/Payout.hpp"
#include "Records.hpp"
#include "test_framework.hpp"

using namespace lockerhub;
using namespace testrecords;

TEST(Payout, PaysForEveryPickedUpParcel) {
  EXPECT_EQ(partnerPayoutCents({pickedUp("P1", 0, 10), pickedUp("P2", 5, 90)}), 80);
}

TEST(Payout, PaysForParcelsHandedBackToTheCarrier) {
  EXPECT_EQ(partnerPayoutCents({returnedToSender("P1", 0, 3000)}), 25);
  EXPECT_EQ(partnerPayoutCents({pickedUp("P1", 0, 10), returnedToSender("P2", 0, 3000), pickedUp("P3", 0, 20)}), 105);
}

TEST(Payout, ParcelsStillAtTheStationAreNotPaidYet) {
  EXPECT_EQ(partnerPayoutCents({inLocker("P1", 0), waiting("P2")}), 0);
  EXPECT_EQ(partnerPayoutCents({}), 0);
}

TEST(Payout, FormatsDollarsAndCents) {
  EXPECT_EQ(formatMoney(650), "$6.50");
  EXPECT_EQ(formatMoney(5), "$0.05");
  EXPECT_EQ(formatMoney(0), "$0.00");
  EXPECT_EQ(formatMoney(12340), "$123.40");
}
