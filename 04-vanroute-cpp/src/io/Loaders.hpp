#pragma once

// Turns parsed CSV rows into domain objects. File formats (one record per line, '#' comments allowed):
//   roads.csv    from,to,minutes                      (two-way road)
//   vans.csv     id,depot,capacity,shiftStart,shiftEnd (times as HH:MM)
//   parcels.csv  id,destination,size,deadline,status   (deadline HH:MM; status "ready" or "cancelled")

#include <vector>

#include "dispatch/Van.hpp"
#include "graph/RoadGraph.hpp"
#include "io/Csv.hpp"
#include "model/Parcel.hpp"

namespace vanroute {

RoadGraph loadRoads(const std::vector<CsvRow>& rows);
std::vector<Van> loadVans(const std::vector<CsvRow>& rows);
std::vector<Parcel> loadParcels(const std::vector<CsvRow>& rows);

}  // namespace vanroute
