# Returns Engine: AI-Assisted Debugging Assessment (C++)

**Practice set #2 · Difficulty: Medium · Suggested time: 90 minutes**

## Scenario

You've joined the team that owns the **Returns Engine**, the service that decides whether a customer's
return request is approved and how much they get refunded. It's a C++17 library with a small CLI:

- a hand-written **JSON parser** that reads the policy and data files,
- a **rule engine** (category, final sale, quantity, return window, abuse velocity, restocking fee),
- repositories, a service layer that applies decisions and notifies customers, and a report.

The previous on-call engineer left 10 tickets. Each ticket has **one root cause**. Find and fix all 10.
Some bugs are not in the obvious policy logic.

## Rules

- **Don't edit the tests or `data/expected_report.txt`.** They are the spec.
- Fix root causes. The real assessment also runs hidden tests.
- **AI assistant:** open a terminal in this folder and run `claude`. It follows `CLAUDE.md`: it can
  point you to the files and functions involved in a flow and answer any C++ or tooling question. It will
  not write code, confirm your guesses, or tell you what the bug is.
- The tickets are listed roughly easiest to hardest. Work them in any order.

## Build and run

Requires CMake 3.16+ and a C++17 compiler (tested with MinGW g++ 13 + Ninja).

```powershell
cmake -S . -B build -G Ninja      # configure (once)
cmake --build build               # build (after every change)
.\build\unit_tests.exe            # run all tests
.\build\unit_tests.exe Velocity   # run tests whose "Suite.Name" contains "Velocity"
.\build\returns-cli.exe data      # process data/requests.json and print the report
```

No Ninja? Drop `-G Ninja` and CMake picks a default generator.

## Architecture

```
src/
  util/      Date (parse, day arithmetic), Money (cents, percentOf, formatCents), Strings
  json/      Json.hpp/.cpp: JSON Value + recursive-descent parser
  model/     Models (Customer, Order, LineItem, ReturnRequest, Decision) + JSON loaders
  policy/    PolicyConfig: default + per-category policies, tier bonuses, fee waivers
  engine/    Rule base class, concrete Rules, ReturnEngine + makeDefaultRules()
  fraud/     ReturnVelocityTracker: returns per customer in a rolling window
  store/     OrderRepository, CustomerRepository
  service/   ReturnService: decide -> apply -> audit -> notify
  report/    runReport(): loads data/, runs every request, formats the report
app/main.cpp returns-cli entry point
tests/       tiny GoogleTest-style framework + unit and end-to-end tests
data/        policies.json, customers.json, orders.json, requests.json, expected_report.txt
```

## Tickets

| ID | Area | Report |
|----|------|--------|
| RP-201 | Dates | The CLI refuses to load our order history: one old order has a date the parser says can't exist. Day counts across February look off for some years too. |
| RP-202 | Window | Customers returning on the **last day** of their window are refused. Agents are overriding by hand. |
| RP-203 | Config | Since someone edited the data files on Windows, `returns-cli` won't start. |
| RP-204 | Fees | Finance: restocking fees are never charged, so we refund the full price. Fees must be rounded to the nearest cent, half up. |
| RP-205 | Policy | Ops raised the default restocking fee to 10%, but apparel returns still have no fee. |
| RP-206 | Over-returns | A customer returned the same cable more times than they bought it. |
| RP-207 | Abuse | The return-velocity limit is inconsistent: some heavy returners get through and others are blocked early. Batches are replayed out of date order. |
| RP-208 | Encoding | Names with accents (for example "Crème Brûlée at Home") come out garbled in the report. |
| RP-209 | Notifications | Customers stopped getting return-decision emails (the outbox is empty), but the audit log looks fine. |
| RP-210 | Final sale | Final-sale items are being refunded. `FinalSaleRule` has its own unit test, and it passes. |

## Done when

- `.\build\unit_tests.exe` reports **58 tests, all passed**.
- `.\build\returns-cli.exe data` prints the same report as `data/expected_report.txt`.

Tip: run `git init && git add -A && git commit -m baseline` before you start, so you can `git diff` your work
or reset and retry later.

Answer key: `../_answer_keys/oa-02-returns-cpp.md.b64` (base64 so you can't spoil it by accident):

```powershell
[Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((Get-Content -Raw ..\_answer_keys\oa-02-returns-cpp.md.b64)))
```
