# ShiftPlan: AI-Assisted Coding Assessment (C++)

**Practice set #6 · Difficulty: Hard · Suggested time: 120 minutes · Focus: building a feature**

## Scenario

ShiftPlan plans the night shift at an Amazon sort center's outbound dock. Every evening the shift lead loads the night's work: trailers to unload, lanes to sort and outbound trucks to load. Each job takes a known number of minutes, needs a crew of a certain type (unloader, sorter, loader) and can only start once the jobs it depends on are done. Each outbound truck has a cutoff: once its load job is done, the trailer takes 15 minutes to seal, and a truck that isn't sealed by its cutoff misses its line-haul slot. Trucks that leave early in the shift are *rush trucks*, and ops rule PRI-2 puts the work they need ahead of everything else. From the plan, the dock gets its departure board, the transportation team sees which trucks will be late, and site leadership tracks how much of the paid crew time is spent working.

The codebase is a C++17 library with a CLI and no third-party dependencies. A run flows like this:
- **Load:** `loadSite` reads four CSV files (shift, crews, jobs, trucks).
- **Rush boost:** `applyRushBoost` raises the priority of rush work (rule PRI-2).
- **Plan:** the planner (`src/plan/Scheduler.cpp`) produces the dispatch board, the critical path, a cross-training what-if and the shift's crew timeline.
- **Report:** the departure board and the utilization metrics are computed from the finished plan, and the report prints it all.

Plan times are minutes since the shift started; the files and the report use clock times, and the night shift crosses midnight. Tests use a small GoogleTest-style framework, plus an end-to-end test that compares the CLI report with a known-good report.

You've joined the team that owns ShiftPlan. The planner itself was never written: every function is a placeholder, so tonight's report can't plan anything. Three tickets from the last pilot site are also open. There are two parts, and **most of the work is Part A**:

- **Part A: build a feature.** Implement the shift planner, level by level.
- **Part B: fix 3 bugs.** Each ticket has **one root cause**.

## Rules

- **Don't edit the tests or `data/expected_report.txt`.** They are the spec. The real assessment also runs hidden tests.
- Fix root causes.
- **AI assistant:** open a terminal in this folder and run `claude`. It follows `CLAUDE.md`. It can point you to files and functions, answer any C++/tooling question, and explain a data structure or algorithm *you name*. It will not write code, design your approach, confirm your guesses, or tell you what a bug is.

## Build and run

Requires CMake 3.16+ and a C++17 compiler (tested with MinGW g++ 13 + Ninja).

```powershell
cmake -S . -B build -G Ninja        # configure (once)
cmake --build build                 # build (after every change)
.\build\unit_tests.exe              # run all tests
.\build\unit_tests.exe Level1       # run one Part A level (Level1 .. Level4)
.\build\unit_tests.exe Departures   # run tests whose "Suite.Name" contains "Departures"
.\build\shiftplan-cli.exe data      # plan data/ and print the shift report
```

No Ninja? Drop `-G Ninja` and CMake picks a default generator.

## Part A: Shift planner

`src/plan/Scheduler.cpp` holds placeholders for six functions. The report already calls all of them. Right now the dispatch board is just the file order, there is no critical path or what-if, and every job is carried over to the next shift.

Build them in four levels. Each level builds on the previous one and never changes what an earlier level's tests expect, so get each level green before moving on:

| Level | Functions | What you add | Tests |
|-------|-----------|--------------|-------|
| 1 | `dispatchOrder`, `blockedJobs` | Validate the jobs; the dispatch board (prerequisites first, then priority and id); find the jobs that can never start | `unit_tests.exe Level1` (9) |
| 2 | `earliestTimes`, `criticalPath` | Timing if every job had its own crew; the chain of jobs that decides when the work is done | `unit_tests.exe Level2` (6, including a performance test) |
| 3 | `scheduleCrews` | A limited number of interchangeable crews, simulated moment by moment | `unit_tests.exe Level3` (7) |
| 4 | `planShift` | Crew types, and nothing may run past the end of the shift: work that can't finish is carried over, along with everything that needs it | `unit_tests.exe Level4` (7, including a performance test) |

The full spec (numbered rules 1-13, every tie-break, the performance requirement) is the comment in `src/plan/Scheduler.hpp`. The performance tests use a 100,000-job site; they are timed in the default Debug build.

## Part B: Tickets

The unit tests for these tickets don't need Part A. The sample-shift numbers below are what the CLI report shows once the planner works.

| ID | Area | Report |
|----|------|--------|
| SP-1 | Dock operations | Since the pilot site moved to nights, the departure board flags trucks as **LATE** that were sealed with plenty of time to spare. Tonight: T1, T3 and T4 (T3 sealed at 01:40 for a 03:00 cutoff). T2 is always shown correctly. |
| SP-2 | Site leadership | Crew utilization in the report doesn't match the labor-planning sheet. Using the report's own plan, labor planning gets **31.7%** of paid crew time worked; the report says 30.6%. The gap gets bigger on nights when a small team is very busy while a large one is idle. |
| SP-3 | Transportation | Rush truck T1 (cutoff 00:40) was sealed at **02:05** tonight, although rule PRI-2 is supposed to put everything a rush truck needs ahead of regular work. The report lists T1's jobs as boosted. T2, the other rush truck, left on time. |

## Architecture

```
src/
  model/   Job + CrewPool, Shift (clock time <-> minutes since shift start), Truck
  io/      Csv (parseCsv, readCsv), Clock (parseClock, formatClock), Loaders (shift, crews, jobs, trucks; loadSite)
  plan/    RushBoost (rule PRI-2), Scheduler: the planner (Part A)
  report/  Departures (departure board, rule OUT-3), Metrics (team/crew utilization, trucks on time), Report
app/main.cpp shiftplan-cli entry point
tests/     tiny GoogleTest-style framework; JobBuilders.hpp (compact jobs, big-site generator); unit, level and end-to-end tests
data/      shift.csv, crews.csv, jobs.csv, trucks.csv, expected_report.txt
```

## Done when

- `.\build\unit_tests.exe` reports **49 tests, all passed**.
- `.\build\shiftplan-cli.exe data` prints the same report as `data/expected_report.txt`.

Answer key (bugs + a reference solution for Part A): `../_answer_keys/06-shiftplan-cpp.md.b64`. It's local only and base64-encoded so you can't spoil it by accident:

```powershell
[Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((Get-Content -Raw ..\_answer_keys\06-shiftplan-cpp.md.b64)))
```
