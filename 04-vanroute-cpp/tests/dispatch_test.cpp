#include <stdexcept>
#include <vector>

#include "TableOracle.hpp"
#include "dispatch/DispatchQueue.hpp"
#include "dispatch/Planner.hpp"
#include "dispatch/Van.hpp"
#include "test_framework.hpp"

using namespace vanroute;

// Every test below uses its own node names (prefix per test) so results can't leak between tests.

namespace {

Parcel parcel(std::string id, std::string destination, int size, int deadline, bool cancelled = false) {
  Parcel p;
  p.id = std::move(id);
  p.destination = std::move(destination);
  p.size = size;
  p.deadline = deadline;
  p.cancelled = cancelled;
  return p;
}

}  // namespace

// ---------- Van ----------

TEST(Van, StoresEveryField) {
  const Van v("V9", "HUB", 8, 7 * 60, 15 * 60 + 30);
  EXPECT_EQ(v.id, std::string("V9"));
  EXPECT_EQ(v.depot, std::string("HUB"));
  EXPECT_EQ(v.capacity, 8);
  EXPECT_EQ(v.shiftStart, 420);
  EXPECT_EQ(v.shiftEnd, 930);
}

TEST(Van, ValidatesCapacityAndShift) {
  EXPECT_THROW(Van("V", "D", 0, 480, 600), std::invalid_argument);
  EXPECT_THROW(Van("V", "D", 4, 600, 600), std::invalid_argument);
  EXPECT_NO_THROW(Van("V", "D", 4, 480, 481));
}

// ---------- DispatchQueue ----------

TEST(DispatchQueue, EarliestDeadlineFirst) {
  DispatchQueue q;
  q.push(parcel("P1", "X", 1, 600));
  q.push(parcel("P2", "X", 1, 500));
  q.push(parcel("P3", "X", 1, 700));
  q.push(parcel("P4", "X", 1, 450));
  std::vector<std::string> order;
  while (!q.empty()) order.push_back(q.pop().id);
  EXPECT_EQ(order, (std::vector<std::string>{"P4", "P2", "P1", "P3"}));
}

TEST(DispatchQueue, TiesByParcelId) {
  DispatchQueue q;
  q.push(parcel("P9", "X", 1, 600));
  q.push(parcel("P1", "X", 1, 600));
  q.push(parcel("P5", "X", 1, 600));
  std::vector<std::string> order;
  while (!q.empty()) order.push_back(q.pop().id);
  EXPECT_EQ(order, (std::vector<std::string>{"P1", "P5", "P9"}));
}

TEST(DispatchQueue, SameDeadlineLargerParcelsFirst) {
  DispatchQueue q;
  q.push(parcel("P1", "X", 1, 600));
  q.push(parcel("P2", "X", 3, 600));
  q.push(parcel("P3", "X", 2, 600));
  q.push(parcel("P0", "X", 1, 540));
  q.push(parcel("P4", "X", 3, 600));
  std::vector<std::string> order;
  while (!q.empty()) order.push_back(q.pop().id);
  EXPECT_EQ(order, (std::vector<std::string>{"P0", "P2", "P4", "P3", "P1"}));
}

// ---------- Planner (uses a table of travel times, not the road network) ----------

TEST(Planner, DeliversInDispatchOrder) {
  TableOracle t;
  t.set("do-D", "do-A", 10).set("do-D", "do-B", 20).set("do-A", "do-B", 15);
  const Planner planner(t);
  const std::vector<Van> vans{Van("V1", "do-D", 10, 480, 1080)};

  // B has the earlier deadline, so it is visited first even though A is closer.
  const Plan plan = planner.plan(vans, {parcel("PA", "do-A", 1, 700), parcel("PB", "do-B", 1, 600)});

  ASSERT_EQ(plan.vans[0].stops.size(), static_cast<std::size_t>(2));
  EXPECT_EQ(plan.vans[0].stops[0].parcelId, std::string("PB"));
  EXPECT_EQ(plan.vans[0].stops[1].parcelId, std::string("PA"));
  EXPECT_EQ(plan.vans[0].stops[0].arrival, 500);  // 08:00 + 20
}

TEST(Planner, LaterStopsIncludeTimeSpentAtEarlierStops) {
  TableOracle t;
  t.set("eta-D", "eta-A", 10).set("eta-A", "eta-B", 10).set("eta-B", "eta-C", 10);
  const Planner planner(t);  // 5 minutes at every stop
  const Plan plan = planner.plan({Van("V1", "eta-D", 10, 480, 1080)},
                                 {parcel("PA", "eta-A", 1, 900), parcel("PB", "eta-B", 1, 901), parcel("PC", "eta-C", 1, 902)});

  ASSERT_EQ(plan.vans[0].stops.size(), static_cast<std::size_t>(3));
  EXPECT_EQ(plan.vans[0].stops[0].arrival, 490);  // 08:10
  EXPECT_EQ(plan.vans[0].stops[1].arrival, 505);  // 08:10 + 5 at A + 10 driving
  EXPECT_EQ(plan.vans[0].stops[2].arrival, 520);  // + 5 at B + 10 driving
}

TEST(Planner, ArrivingExactlyAtTheDeadlineIsOnTime) {
  TableOracle t;
  t.set("ot-D", "ot-A", 30);
  const Planner planner(t);
  const Plan plan = planner.plan({Van("V1", "ot-D", 5, 480, 1080)}, {parcel("P1", "ot-A", 1, 510)});
  ASSERT_EQ(plan.vans[0].stops.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(plan.vans[0].stops[0].arrival, 510);
  EXPECT_FALSE(plan.vans[0].stops[0].late);
}

TEST(Planner, FillsVansInOrderAndRespectsCapacity) {
  TableOracle t;
  t.set("cap-D", "cap-A", 5);
  const Planner planner(t);
  const std::vector<Van> vans{Van("V1", "cap-D", 3, 480, 1080), Van("V2", "cap-D", 3, 480, 1080)};

  const Plan plan = planner.plan(vans, {parcel("P1", "cap-A", 2, 600), parcel("P2", "cap-A", 2, 600),
                                        parcel("P3", "cap-A", 1, 600), parcel("P4", "cap-A", 2, 600)});

  EXPECT_EQ(plan.vans[0].load, 3);  // P1 + P3
  EXPECT_EQ(plan.vans[1].load, 2);  // P2
  ASSERT_EQ(plan.unassigned.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(plan.unassigned[0].parcelId, std::string("P4"));
  EXPECT_CONTAINS(plan.unassigned[0].reason, "capacity");
}

TEST(Planner, IgnoresCancelledParcels) {
  TableOracle t;
  t.set("cx-D", "cx-A", 5);
  const Planner planner(t);
  const Plan plan = planner.plan({Van("V1", "cx-D", 5, 480, 1080)},
                                 {parcel("P1", "cx-A", 1, 600, true), parcel("P2", "cx-A", 1, 600)});
  ASSERT_EQ(plan.vans[0].stops.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(plan.vans[0].stops[0].parcelId, std::string("P2"));
  EXPECT_TRUE(plan.unassigned.empty());
}

TEST(Planner, UnreachableDestinationsAreUnassigned) {
  TableOracle t;
  t.set("ur-D", "ur-A", 5);
  const Planner planner(t);
  const Plan plan = planner.plan({Van("V1", "ur-D", 5, 480, 1080)}, {parcel("P1", "ur-ISLAND", 1, 600)});
  ASSERT_EQ(plan.unassigned.size(), static_cast<std::size_t>(1));
  EXPECT_CONTAINS(plan.unassigned[0].reason, "unreachable");
}

TEST(Planner, FlagsLateStops) {
  TableOracle t;
  t.set("late-D", "late-A", 40);
  const Planner planner(t);
  const Plan plan = planner.plan({Van("V1", "late-D", 5, 480, 1080)}, {parcel("P1", "late-A", 1, 510)});
  ASSERT_EQ(plan.vans[0].stops.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(plan.vans[0].stops[0].arrival, 520);
  EXPECT_TRUE(plan.vans[0].stops[0].late);
}

TEST(Planner, NeverSchedulesPastTheEndOfAShift) {
  TableOracle t;
  t.set("sh-D", "sh-A", 20).set("sh-A", "sh-B", 20);
  const Planner planner(t);
  // Shift 08:00-08:40: A is done at 08:25, B would be delivered 08:45 + 5 > 08:40.
  const Plan plan = planner.plan({Van("V1", "sh-D", 5, 480, 520)},
                                 {parcel("PA", "sh-A", 1, 600), parcel("PB", "sh-B", 1, 600)});
  ASSERT_EQ(plan.vans[0].stops.size(), static_cast<std::size_t>(1));
  ASSERT_EQ(plan.unassigned.size(), static_cast<std::size_t>(1));
  EXPECT_EQ(plan.unassigned[0].parcelId, std::string("PB"));
  EXPECT_CONTAINS(plan.unassigned[0].reason, "shift");
}

TEST(Planner, UsesCurrentTravelTimesAfterTheMapChanges) {
  TableOracle t;
  t.set("rc-D", "rc-A", 10);
  const Planner planner(t);
  const std::vector<Van> vans{Van("V1", "rc-D", 5, 480, 1080)};
  const std::vector<Parcel> parcels{parcel("P1", "rc-A", 1, 900)};

  EXPECT_EQ(planner.plan(vans, parcels).vans[0].stops[0].arrival, 490);

  t.set("rc-D", "rc-A", 45);  // road closure: the detour takes longer
  EXPECT_EQ(planner.plan(vans, parcels).vans[0].stops[0].arrival, 525);

  TableOracle other;  // a different planner with its own map
  other.set("rc-D", "rc-A", 3);
  EXPECT_EQ(Planner(other).plan(vans, parcels).vans[0].stops[0].arrival, 483);
}
