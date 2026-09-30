# VanRoute: AI-Assisted Coding Assessment (C++)

**Practice set #4 · Difficulty: Medium · Suggested time: 90 minutes**

## Scenario

VanRoute plans the day for a last-mile delivery station. Every morning dispatchers load the local road network, the vans on shift (capacity and shift hours) and today's parcels (destination, size, delivery deadline, and whether it was cancelled). VanRoute decides which van carries which parcel, in what order, and when each one arrives. Drivers get their stop lists from its report, and the station manager watches the late-delivery count and how full the fleet is.

It's a C++17 library with a CLI and no third-party dependencies. The same library also runs inside a long-lived planning service that reloads the road map during the day when roads close.
- **Data:** CSV files.
- **Road network:** a graph of named junctions joined by two-way roads, measured in minutes.
- **Planner:** greedy. It takes parcels in order of urgency and gives each one to the first van that has room, can reach the address, and can finish before its shift ends.
- **Travel times:** they come through an oracle interface, so tests can plug in a fake table of times instead of the real network.
- **Tests:** a small GoogleTest-style framework, plus an end-to-end test that compares the CLI report with a known-good report.

Right now the station can't route anything, because travel times over the road network were never implemented. On top of that, five tickets are open. There are two parts:

- **Part A: build a feature.** Compute real travel times over the road network.
- **Part B: fix 5 bugs.** Each ticket has **one root cause**.

## Rules

- **Don't edit the tests or `data/expected_report.txt`.** They are the spec.
- Fix root causes. The real assessment also runs hidden tests.
- **AI assistant:** open a terminal in this folder and run `claude`. It follows `CLAUDE.md`. It can point you to files and functions, answer any C++/tooling question, and explain a data structure or algorithm *you name*. It will not write code, design your approach, confirm your guesses, or tell you what a bug is.

## Build and run

Requires CMake 3.16+ and a C++17 compiler (tested with MinGW g++ 13 + Ninja).

```powershell
cmake -S . -B build -G Ninja        # configure (once)
cmake --build build                 # build (after every change)
.\build\unit_tests.exe              # run all tests
.\build\unit_tests.exe Planner      # run tests whose "Suite.Name" contains "Planner"
.\build\vanroute-cli.exe data       # plan data/ and print the dispatch report
```

No Ninja? Drop `-G Ninja` and CMake picks a default generator.

## Part A: Travel times

The planner asks `RoadNetworkOracle` "how many minutes from A to B?", and the oracle gets its answers from `shortestTimes` in `src/graph/ShortestPath.cpp`. That function is a placeholder: it says every node except the start is unreachable, so the CLI currently reports every parcel as "destination unreachable".

Implement it. The full spec (return value, unreachable nodes, zero-minute and parallel roads, number sizes, performance requirement) is in `src/graph/ShortestPath.hpp`. The examples are in `tests/shortest_path_test.cpp`.

## Part B: Tickets

| ID | Area | Report |
|----|------|--------|
| VR-1 | Driver scorecards | Drivers who arrive exactly at the promised time are being marked **LATE** on their scorecards. Today it happened to P10. |
| VR-2 | Cancellations | Customers who cancelled their order still get a van at their door. Support confirms those parcels are marked as cancelled in `parcels.csv`. |
| VR-3 | Dispatch | On busy mornings the bulky parcels are the ones left behind at the station, even though ops rule DSP-7 is supposed to prevent exactly that. |
| VR-4 | Reporting | The station manager says the fleet utilization figure looks too good: on days when a van never leaves the depot, the number doesn't go down. |
| VR-5 | ETAs | Customers say drivers arrive later than the ETA we send them, and the gap grows the later the stop is in the route. |

## Architecture

```
src/
  io/        Csv (parseCsv, readCsv, toInt, parseClock/formatClock), Loaders (roads, vans, parcels)
  model/     Parcel
  graph/     RoadGraph (named nodes, two-way roads), ShortestPath (Part A)
  dispatch/  Van, DispatchQueue (dispatch order), TravelOracle (+ RoadNetworkOracle), Planner (greedy assignment)
  report/    fleetUtilization, lateCount, renderReport, runReport
app/main.cpp vanroute-cli entry point
tests/       tiny GoogleTest-style framework; TableOracle test double; unit + end-to-end tests
data/        roads.csv, vans.csv, parcels.csv, expected_report.txt
```

## Done when

- `.\build\unit_tests.exe` reports **38 tests, all passed**.
- `.\build\vanroute-cli.exe data` prints the same report as `data/expected_report.txt`.

Answer key (bugs + a reference solution for Part A): `../_answer_keys/04-vanroute-cpp.md.b64`. It's local only and base64-encoded so you can't spoil it by accident:

```powershell
[Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((Get-Content -Raw ..\_answer_keys\04-vanroute-cpp.md.b64)))
```
