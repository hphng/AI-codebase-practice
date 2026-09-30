#pragma once

#include <string>

#include "io/Loaders.hpp"

namespace shiftplan {

// The shift report: rush trucks (PRI-2), the dispatch board, the critical path, a cross-training
// what-if, the crew timeline, carried-over work, the departure board and the summary figures.
std::string renderReport(const SiteData& site);

// Loads the site from `dataDir` (see io/Loaders.hpp) and renders the report.
std::string runReport(const std::string& dataDir);

}  // namespace shiftplan
