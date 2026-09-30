#pragma once

#include <string>
#include <vector>

#include "model/Compartment.hpp"
#include "model/Event.hpp"
#include "model/StationConfig.hpp"

namespace lockerhub {

// File formats (all CSV with a header row; see data/ for examples):
//
//   station.csv  station,name,hold_hours           one row; hold_hours > 0
//   lockers.csv  compartment,size                  size is S, M or L; ids are unique
//   events.csv   day,time,event,parcel,size        event is DEPOSIT, PICKUP or CLOSE
//                                                  size is required for DEPOSIT and empty otherwise
//                                                  rows are in time order
//
// Every loader throws std::runtime_error naming the file's row number when a row is invalid.

StationConfig loadStation(const std::string& path);
std::vector<Compartment> loadCompartments(const std::string& path);
std::vector<Event> loadEvents(const std::string& path);

// Same as above, from rows that were already split (header removed). `source` is used in errors.
StationConfig stationFromRows(const std::vector<std::vector<std::string>>& rows, const std::string& source);
std::vector<Compartment> compartmentsFromRows(const std::vector<std::vector<std::string>>& rows,
                                              const std::string& source);
std::vector<Event> eventsFromRows(const std::vector<std::vector<std::string>>& rows, const std::string& source);

}  // namespace lockerhub
