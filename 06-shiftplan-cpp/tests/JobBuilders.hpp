#pragma once

// Small helpers to write jobs compactly in tests, plus a generator for large sites.

#include <cstdint>
#include <string>
#include <vector>

#include "model/Job.hpp"

namespace testjobs {

using shiftplan::Job;
using Ids = std::vector<std::string>;

inline Job job(const std::string& id, int minutes, int priority, Ids after = {}, const std::string& crew = "crew") {
  Job j;
  j.id = id;
  j.minutes = minutes;
  j.priority = priority;
  j.crew = crew;
  j.after = std::move(after);
  return j;
}

// A deterministic big site: `count` jobs "J000000".., each needing up to 3 of the previous 500 jobs,
// 1-60 minutes, priority 0-99, crew types "a".."d".
inline std::vector<Job> bigSite(int count) {
  std::vector<Job> jobs;
  jobs.reserve(count);
  std::uint32_t seed = 2024;
  auto next = [&seed]() {
    seed = seed * 1664525u + 1013904223u;
    return seed >> 8;
  };
  auto name = [](int i) { return "J" + std::to_string(1000000 + i).substr(1); };
  const char* types[] = {"a", "b", "c", "d"};
  for (int i = 0; i < count; ++i) {
    Job j;
    j.id = name(i);
    j.minutes = 1 + static_cast<int>(next() % 60);
    j.priority = static_cast<int>(next() % 100);
    j.crew = types[next() % 4];
    if (i > 0) {
      const int links = static_cast<int>(next() % 4);  // 0-3
      for (int k = 0; k < links; ++k) {
        const int back = 1 + static_cast<int>(next() % 500);
        if (i - back >= 0) j.after.push_back(name(i - back));
      }
    }
    jobs.push_back(std::move(j));
  }
  return jobs;
}

}  // namespace testjobs
