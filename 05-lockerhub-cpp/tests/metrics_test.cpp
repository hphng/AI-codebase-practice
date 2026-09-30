#include "report/Metrics.hpp"
#include "Records.hpp"
#include "test_framework.hpp"

using namespace lockerhub;
using namespace testrecords;

TEST(Metrics, PickupRateOfFinishedParcels) {
  EXPECT_EQ(pickupRatePercent({pickedUp("P1", 0, 10), pickedUp("P2", 0, 10), pickedUp("P3", 0, 10),
                               returnedToSender("P4", 0, 3000)}),
            75.0);
}

TEST(Metrics, PickupRateLeavesOutParcelsStillAtTheStation) {
  // 3 of the 4 parcels that are done were picked up; the other two aren't done yet.
  EXPECT_EQ(pickupRatePercent({pickedUp("P1", 0, 10), inLocker("P2", 0), pickedUp("P3", 0, 10), waiting("P4"),
                               returnedToSender("P5", 0, 3000), pickedUp("P6", 0, 10)}),
            75.0);
}

TEST(Metrics, PickupRateIsZeroWhenNothingIsFinished) {
  EXPECT_EQ(pickupRatePercent({}), 0.0);
  EXPECT_EQ(pickupRatePercent({returnedToSender("P1", 0, 3000)}), 0.0);
}

TEST(Metrics, AverageDwellOfPickedUpParcels) {
  // 100 and 251 minutes: the average 175.5 rounds down to 175.
  EXPECT_EQ(averageDwellMinutes({pickedUp("P1", 0, 100), pickedUp("P2", 49, 300), returnedToSender("P3", 0, 9000),
                                 inLocker("P4", 0)})
                .value_or(-1),
            175);
}

TEST(Metrics, AverageDwellNeedsAPickup) {
  EXPECT_FALSE(averageDwellMinutes({inLocker("P1", 0), waiting("P2")}).has_value());
}
