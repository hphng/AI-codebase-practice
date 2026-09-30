#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "model/Job.hpp"

namespace shiftplan {

// Thrown when the jobs can't be planned (bad ids, bad durations, unknown prerequisites, cycles, ...).
class PlanError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

// One job placed in time. Times are minutes since the shift started.
struct Slot {
  std::string job;
  int start = 0;
  int finish = 0;
  int crew = 0;  // crew number (1..n within the job's crew type); 0 when crews don't matter (level 2)
};

struct ShiftPlan {
  std::vector<Slot> slots;               // jobs that run this shift
  std::vector<std::string> carriedOver;  // jobs left for the next shift, sorted by id
};

// PART A: implement the shift planner (src/plan/Scheduler.cpp).
//
// A sort-center shift is a set of jobs (unload a trailer, sort a lane, load a truck, ...). A job can
// only start once every job in its `after` list has finished. The report (src/report/Report.cpp)
// shows the dispatch board, the critical path, a cross-training what-if and the crew timeline, all
// computed here.
//
// It is built in four levels. Each level's examples are in tests/scheduler_level<N>_test.cpp; run one
// level with `unit_tests.exe Level1` (Level2, ...). Later levels build on earlier ones and never
// change what an earlier level's tests expect.
//
// ---- Level 1: dispatch order --------------------------------------------------------------------
//  1. Every function first checks the jobs and throws PlanError if a job has an empty id, an id that
//     another job also has, minutes <= 0, a prerequisite that isn't one of the jobs, or itself as a
//     prerequisite. Listing the same prerequisite twice is allowed and means the same as once.
//  2. dispatchOrder(jobs): every job id exactly once, each job after all of its prerequisites. Of the
//     jobs that could come next, the one with the highest priority comes next; ties go to the lowest
//     id (plain string comparison).
//  3. If the prerequisites form a cycle there is no valid order, and dispatchOrder throws PlanError.
//  4. blockedJobs(jobs): the ids of the jobs that can never start, because they are on a cycle or
//     need (directly or through other jobs) a job that is. Sorted by id; empty if there is no cycle.
//
// ---- Level 2: timing with unlimited crews -------------------------------------------------------
//  In this level every job gets a crew of its own, so only the prerequisites hold a job back.
//  5. earliestTimes(jobs): one Slot per job, in dispatchOrder's order. A job starts when its last
//     prerequisite finishes (at 0 if it has none), finishes `minutes` later, and has crew 0.
//  6. criticalPath(jobs): the chain of jobs that decides when all the work is done. Start at the job
//     with the latest finish (ties: lowest id), then repeatedly step to that job's prerequisite with
//     the latest finish (ties: lowest id), until reaching a job without prerequisites. Returned in
//     working order (first job first). No jobs: empty path.
//  7. Both throw PlanError on a cycle, like dispatchOrder.
//
// ---- Level 3: a limited number of crews ---------------------------------------------------------
//  8. scheduleCrews(jobs, crews): there are `crews` identical crews, numbered 1..crews, and any crew
//     can do any job (Job::crew is ignored in this level). crews < 1 throws std::invalid_argument. The
//     schedule is built moment by moment, starting at time 0:
//       a. first, every job that finishes at this moment is done: its crew is free again, and each job
//          whose prerequisites are now all done becomes ready;
//       b. then, while there is a free crew and a ready job, the ready job with the highest priority
//          (ties: lowest id) starts now, on the free crew with the lowest number;
//       c. the next moment is the next time a running job finishes.
//     So a crew never sits idle while a job is ready.
//  9. The result has one Slot per job, sorted by start, then by crew number. A cycle throws PlanError.
//
// ---- Level 4: crew types and the end of the shift, at site scale --------------------------------
// 10. planShift(jobs, crews, shiftMinutes): each job needs a crew of its own type (Job::crew); `crews`
//     says how many crews each type has, numbered 1..n within the type. Rule 8 applies per type: a free
//     crew only takes ready jobs of its own type. Throws PlanError if a job's type isn't in `crews`
//     (or on a cycle), std::invalid_argument if a crew count or shiftMinutes is < 1.
// 11. Nothing may run past the end of the shift: a job may only start at time t if
//     t + minutes <= shiftMinutes. A ready job that can't finish in time is never started. It is carried
//     over to the next shift and doesn't hold its crew, which moves on to the next ready job. Every job
//     that needs a carried-over job (directly or through other jobs) is carried over too.
// 12. ShiftPlan::slots is sorted by start, then crew type, then crew number. ShiftPlan::carriedOver
//     holds the ids of the carried-over jobs, sorted.
// 13. Performance: a big site plans up to 100,000 jobs with 300,000 prerequisite links, a handful of
//     crew types and hundreds of crews. Every function must scale close to linearly (a log factor is
//     fine); nothing may rescan all jobs or all crews at every step. There are performance tests in
//     tests/scheduler_level2_test.cpp and tests/scheduler_level4_test.cpp.

// Level 1
std::vector<std::string> dispatchOrder(const std::vector<Job>& jobs);
std::vector<std::string> blockedJobs(const std::vector<Job>& jobs);

// Level 2
std::vector<Slot> earliestTimes(const std::vector<Job>& jobs);
std::vector<std::string> criticalPath(const std::vector<Job>& jobs);

// Level 3
std::vector<Slot> scheduleCrews(const std::vector<Job>& jobs, int crews);

// Level 4
ShiftPlan planShift(const std::vector<Job>& jobs, const CrewPool& crews, int shiftMinutes);

}  // namespace shiftplan
