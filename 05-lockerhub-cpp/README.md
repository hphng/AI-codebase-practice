# LockerHub: AI-Assisted Coding Assessment (C++)

**Practice set #5 · Difficulty: Easy · Suggested time: 90 minutes · Focus: building a feature**

## Scenario

LockerHub runs Amazon Hub Locker stations: banks of self-service lockers in partner stores where couriers drop off parcels and customers collect them with a code. Every station has small, medium and large compartments. A courier scans a parcel and a door pops open; the customer has a fixed pickup window (48 hours at the sample station) before the parcel goes back to the carrier. When the lockers are full, parcels wait in the store's back room until a door frees up. The hosting store is paid per parcel it handles, customers get a reminder when a parcel has sat for a day, and area managers watch each station's pickup rate.

The codebase is a C++17 library with a CLI and no third-party dependencies.
- **Data:** three CSV files: the station (`station.csv`), its compartments (`lockers.csv`) and the event feed of deposits, pickups and closing time (`events.csv`).
- **Locker engine:** `LockerBank` decides which door each parcel gets, frees doors, returns unclaimed parcels and runs the waitlist. It hides its data behind a pimpl, so all of its state lives in one `.cpp` file.
- **Station replay:** `runDay` feeds the events through the engine and keeps a `ParcelRecord` per parcel (waiting, in a locker, picked up, returned).
- **Downstream modules:** reminders (`notify/`), the partner payout (`billing/`) and the report metrics (`report/`) only read those records.
- **Tests:** a small GoogleTest-style framework, plus an end-to-end test that compares the CLI report with a known-good report.

You've joined the team that owns LockerHub. The locker engine was never written: the station can't store a single parcel, so the new Capitol Hill station can't open. Three smaller tickets are also open. There are two parts, and **most of the work is Part A**:

- **Part A: build a feature.** Implement the locker engine, level by level.
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
.\build\unit_tests.exe Payout       # run tests whose "Suite.Name" contains "Payout"
.\build\lockerhub-cli.exe data      # replay data/ and print the station report
```

No Ninja? Drop `-G Ninja` and CMake picks a default generator.

## Part A: Locker engine

`LockerBank` in `src/locker/LockerBank.cpp` is a placeholder that says "no space" to every parcel, so the CLI report currently shows every parcel stuck outside. `runDay` (`src/station/StationDay.cpp`) and the report already call it; as soon as it works, the report fills in.

Build it in four levels. Each level adds rules on top of the previous one and never changes what an earlier level's tests expect, so get each level green before moving on:

| Level | What you add | Tests |
|-------|--------------|-------|
| 1 | Store and pick up: choose the right door for each parcel, refuse duplicates, free doors on pickup | `unit_tests.exe Level1` (9) |
| 2 | Queries: where is a parcel, how many doors are free, which parcels have waited longest | `unit_tests.exe Level2` (4) |
| 3 | Pickup window: parcels not collected in time go back to the carrier; time only moves forward | `unit_tests.exe Level3` (7) |
| 4 | Waitlist, at station scale: parcels that don't fit wait for a door, and the whole engine has to handle a 30,000-door station fast | `unit_tests.exe Level4` (7, including a performance test) |

The full spec (numbered rules 1-16, every tie-break, the performance requirement) is the comment in `src/locker/LockerBank.hpp`. You may add any data you like to `struct LockerBank::State` in the `.cpp`. When all four levels pass, `unit_tests.exe StationDay` checks the engine through the station replay.

## Part B: Tickets

The unit tests for these tickets don't need Part A. The sample-day numbers below are what the CLI report shows once the locker engine works.

| ID | Area | Report |
|----|------|--------|
| LH-1 | Partner billing | The store that hosts the Capitol Hill lockers says our payout statement is short. For the sample days they expected **$3.55**: 7 parcels collected by customers and 3 they handed back to the carrier. The report says $2.80. |
| LH-2 | Customer notifications | The customer for P15 says they never got a pickup reminder. Their parcel went into its locker on D2 at 20:00 and was still there when the station closed on D3 at 20:00, a full day later. |
| LH-3 | Ops reporting | Area managers are escalating Capitol Hill for a poor pickup rate: the report says **41.2%**. Ops counted 10 parcels whose stay is over, and customers collected 7 of them. The figure also seems to drop whenever the lockers are busy at closing time. |

## Architecture

```
src/
  model/     Size (S/M/L, fits), Compartment, Event, StationConfig
  io/        Csv (parseCsv, readCsv), Clock (parseStamp/formatStamp "D2 08:30"), Loaders (station, lockers, events)
  locker/    LockerBank: the locker engine (Part A)
  station/   ParcelRecord, StationDay (runDay: replays events through LockerBank, records each parcel's day)
  notify/    Reminders (dueReminders)
  billing/   Payout (partnerPayoutCents, formatMoney)
  report/    Metrics (pickupRatePercent, averageDwellMinutes), Report (renderReport, runReport)
app/main.cpp lockerhub-cli entry point
tests/       tiny GoogleTest-style framework; Records.hpp record builders; unit, level and end-to-end tests
data/        station.csv, lockers.csv, events.csv, expected_report.txt
```

## Done when

- `.\build\unit_tests.exe` reports **51 tests, all passed**.
- `.\build\lockerhub-cli.exe data` prints the same report as `data/expected_report.txt`.

Answer key (bugs + a reference solution for Part A): `../_answer_keys/05-lockerhub-cpp.md.b64`. It's local only and base64-encoded so you can't spoil it by accident:

```powershell
[Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((Get-Content -Raw ..\_answer_keys\05-lockerhub-cpp.md.b64)))
```
