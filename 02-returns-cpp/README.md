# Returns Engine: AI-Assisted Coding Assessment (C++)

**Practice set #2 · Difficulty: Medium · Suggested time: 90 minutes**

## Scenario

You've joined the team that owns the **Returns Engine**, the service that decides whether a customer's
return request is approved and how much they get refunded. It's a C++17 library with a small CLI:

- a hand-written **JSON parser** that reads the policy and data files,
- a **rule engine** (category, final sale, quantity, return window, abuse velocity, restocking fee),
- repositories, a service layer that applies decisions and notifies customers, and a report.

There are two parts:

- **Part A: build a feature.** Implement household detection for the return-abuse limit.
- **Part B: fix 5 bugs.** Each ticket has **one root cause**. Some are not in the obvious policy logic.

## Rules

- **Don't edit the tests or `data/expected_report.txt`.** They are the spec.
- Fix root causes. The real assessment also runs hidden tests.
- **AI assistant:** open a terminal in this folder and run `claude`. It follows `CLAUDE.md`. It can
  point you to files and functions, answer any C++/tooling question, and explain a data structure or
  algorithm *you name*. It will not write code, design your approach, confirm your guesses, or tell you
  what a bug is.

## Build and run

Requires CMake 3.16+ and a C++17 compiler (tested with MinGW g++ 13 + Ninja).

```powershell
cmake -S . -B build -G Ninja      # configure (once)
cmake --build build               # build (after every change)
.\build\unit_tests.exe            # run all tests
.\build\unit_tests.exe Households # run tests whose "Suite.Name" contains "Households"
.\build\returns-cli.exe data      # process data/requests.json and print the report
```

No Ninja? Drop `-G Ninja` and CMake picks a default generator.

## Part A: Households

The abuse limit ("at most 3 approved returns per 30 days") is meant to apply per **household**, because
people open several accounts to get around it. Two customers are in the same household if they share an
address or a payment card, directly or through a chain of other customers.

The service already looks up each customer's household and counts returns per household. The missing
piece is `buildHouseholds` in `src/fraud/Households.cpp`. Right now it's a placeholder that puts every
customer in their own household. Implement it. The full spec (matching rules, which id represents a
household, performance requirement) is in `src/fraud/Households.hpp`. The examples are in
`tests/household_test.cpp`.

## Part B: Tickets

| ID | Area | Report |
|----|------|--------|
| RP-1 | Window | Customers returning on the **last day** of their window are refused. Agents are overriding by hand. |
| RP-2 | Config | Since someone edited the data files on Windows, `returns-cli` won't start. |
| RP-3 | Fees | Finance: restocking fees are never charged, so we refund the full price. Fees must be rounded to the nearest cent, half up. |
| RP-4 | Over-returns | A customer returned the same cable more times than they bought it. |
| RP-5 | Final sale | Final-sale items are being refunded. `FinalSaleRule` has its own unit test, and it passes. |

## Architecture

```
src/
  util/      Date (parse, day arithmetic), Money (cents, percentOf, formatCents), Strings
  json/      Json.hpp/.cpp: JSON Value + recursive-descent parser
  model/     Models (Customer, Order, LineItem, ReturnRequest, Decision) + JSON loaders
  policy/    PolicyConfig: default + per-category policies, tier bonuses, fee waivers
  engine/    Rule base class, concrete Rules, ReturnEngine + makeDefaultRules()
  fraud/     ReturnVelocityTracker (returns per key in a rolling window), Households (Part A)
  store/     OrderRepository, CustomerRepository
  service/   ReturnService: decide -> apply -> notify -> audit
  report/    runReport(): loads data/, runs every request, formats the report
app/main.cpp returns-cli entry point
tests/       tiny GoogleTest-style framework + unit and end-to-end tests
data/        policies.json, customers.json, orders.json, requests.json, expected_report.txt
```

## Done when

- `.\build\unit_tests.exe` reports **69 tests, all passed**.
- `.\build\returns-cli.exe data` prints the same report as `data/expected_report.txt`.

Answer key (bugs + a reference solution for Part A): `../_answer_keys/02-returns-cpp.md.b64`. It's local
only and base64-encoded so you can't spoil it by accident:

```powershell
[Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((Get-Content -Raw ..\_answer_keys\02-returns-cpp.md.b64)))
```
