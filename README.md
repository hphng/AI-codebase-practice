# AI-assisted codebase practice

Practice repositories for Amazon-style **AI-assisted coding OAs**: an unfamiliar codebase, tests as the spec,
and a restricted AI assistant (see each folder's `CLAUDE.md`). Every set has two parts:

- **Part A:** implement an algorithm feature that is already wired into the app (stub + spec + tests,
  including a performance test).
- **Part B:** fix 5 seeded bugs, roughly easy to hard.

| Folder | Stack | Brief | Part A |
|--------|-------|-------|--------|
| [`01-shoplite`](01-shoplite) | MERN | E-commerce store: catalog, cart, orders | "Frequently bought together" (hash-map counting + top-k) |
| [`02-returns-cpp`](02-returns-cpp) | C++17, CMake | Return-policy engine: JSON parser, rule engine, CLI | Household detection (connected components) |
| [`03-pickpath`](03-pickpath) | MERN | Warehouse picking: bins, pick lists, floor map | Pick-route planner (BFS on a grid) |
| [`04-vanroute-cpp`](04-vanroute-cpp) | C++17, CMake | Delivery-van dispatch planner: CSV loaders, greedy planner, report | Travel times (weighted shortest paths) |

Each folder has its own README with setup, tickets and a "done when" checklist.
Start the restricted assistant by running `claude` **inside** a practice folder.

Suggested workflow: make a branch per attempt (`git switch -c attempt/01-shoplite`), then `git diff main` to
review what you changed.
