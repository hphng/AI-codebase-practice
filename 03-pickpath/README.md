# PickPath: AI-Assisted Coding Assessment (MERN)

**Practice set #3 · Difficulty: Medium · Suggested time: 90 minutes**

## Scenario

PickPath is the internal tool associates use on the floor of a fulfilment centre to pick customer orders. Shift leads build **pick lists**: which SKUs to collect, and how much weight the picker's tote can carry. Pickers log in at a floor kiosk by scanning their badge, or from a browser with badge + PIN. They look up where each SKU is stored and walk the aisles to collect the items. Leads also correct bin quantities after cycle counts, and the whole floor relies on those numbers being right.

The codebase is a MERN app:
- **`server/`** is an Express + Mongoose API with four collections: pickers (hashed PINs, picker/lead roles), bins (SKU, aisle, pick-face position, quantity, unit weight), pick lists, and the warehouse floor plan (a character grid of shelving and walkable aisles). Auth is JWT, with a browser login and a trusted kiosk badge login.
- **`client/`** is a React + Vite app. It has a pick-list page that draws the planned route on a map of the floor, and a bins page grouped by aisle where leads enter cycle counts.
- **Tests:** Jest + Supertest against an in-memory MongoDB, plus Vitest for client helpers.

Operations wants pickers to stop wandering, so every pick list needs a planned walking route, and that part was never finished. Meanwhile five tickets have piled up. There are two parts:

- **Part A: build a feature.** Implement the pick-route planner (a shortest-path problem on the floor grid).
- **Part B: fix 5 bugs.** Each ticket has **one root cause**.

## Rules

- **Don't edit the tests.** They are the spec. The real assessment also runs hidden tests.
- **AI assistant:** open a terminal in this folder and run `claude`. It follows `CLAUDE.md`. It can point you to files and functions, answer syntax/library questions, and explain a data structure or algorithm *you name*. It will not write code, design your approach, confirm your guesses, or tell you what a bug is.

## Setup

Requires Node 18+. No MongoDB install is needed: an in-memory MongoDB is started automatically.

```bash
npm run setup     # installs root, server and client dependencies
npm test          # runs server (Jest) + client (Vitest) suites
npm run dev       # starts BOTH: web UI http://localhost:5174  +  API http://localhost:5001
```

**Open the UI at http://localhost:5174.** The ports differ from ShopLite's, so both can run at once. The first run downloads a MongoDB binary (about 600 MB, one time only, shared with the other sets).

Handy commands:

```bash
cd server && npx jest tests/routePlanner.test.js     # one file
cd server && npx jest -t "badge"                      # tests whose name matches
cd client && npx vitest                               # watch mode
```

Demo logins (dev server): badge `LEAD-0001` PIN `1234` (lead), badge `PICK-0042` PIN `4242` (picker).

## Part A: Pick-route planner

On the **Pick lists** tab, **Plan route** calls `POST /api/picklists/:id/route`. It should return the order to visit the bins and the total number of steps, which the page draws on a map of the floor. The endpoint and the UI exist. The planner doesn't: `planPickRoute` in `server/src/services/routePlanner.js` throws `501 Not Implemented`.

Implement it. The full spec (grid format, movement rules, greedy visiting order, tie-breaks, unreachable stops, performance requirement) is in the function's doc comment. The examples are in `server/tests/routePlanner.test.js`.

## Part B: Tickets

| ID | Area | Report |
|----|------|--------|
| PP-1 | Bins (web) | On the Bins page, aisle 10 is listed between aisles 1 and 2. Pickers walk the floor in the wrong order. |
| PP-2 | Bins | When a lead saves a cycle count, the page still shows the old quantity until they refresh. |
| PP-3 | Pick lists | Leads can't create a pick list that exactly fills the tote (for example 3 x 0.1 kg in a 0.3 kg tote). The API says it's over the limit. |
| PP-4 | Security | Pen-test finding (severity HIGH): the kiosk badge login can be tricked into logging in as someone else **without a badge**. |
| PP-5 | Accounts | After changing their PIN, pickers can't log in with the new PIN (or the old one). |

## Architecture

```
server/src
  app.js                 Express app: middleware + route mounting
  routes/index.js        /api/auth, /api/bins, /api/picklists, /api/warehouse
  controllers/           auth (login, kiosk badge, PIN change), bins, pick lists, warehouse
  services/              toteService (weight check), routePlanner (Part A)
  models/                Picker, Bin, PickList, Warehouse (Mongoose)
  middleware/            auth (protect / requireRole), asyncHandler, errorHandler
  seed/seed.js           demo warehouse grid, 16 bins, 2 pickers, 2 pick lists
server/tests             Jest + Supertest + mongodb-memory-server

client/src
  App.jsx                Tabs: pick lists / bins
  utils/bins.js          groupByAisle (+ tests)
  components/            Login, PickLists (route + create), WarehouseMap, Bins (cycle counts)
```

## Done when

- `npm test` prints `server: PASS   client: PASS`.
- `npm run dev` works end to end: log in, see bins grouped by aisle in walking order, correct a count as a lead, create a pick list, and plan a route that's drawn on the map.

Answer key (bugs + a reference solution for Part A): `../_answer_keys/03-pickpath.md.b64`. It's local only and base64-encoded so you can't spoil it by accident:

```powershell
[Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((Get-Content -Raw ..\_answer_keys\03-pickpath.md.b64)))
```
