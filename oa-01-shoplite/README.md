# ShopLite: AI-Assisted Debugging Assessment

**Practice set #1 · Difficulty: Medium · Suggested time: 90 minutes**

## Scenario

You just joined the team that owns **ShopLite**, a small MERN storefront (MongoDB, Express, React, Node).
The previous on-call engineer left 10 open tickets. Each ticket has **one root cause** in the code.
Your job is to find and fix all 10 so the whole test suite passes and the app works end to end.

## Rules

- **Don't edit the tests.** They are the spec. (The real assessment also runs hidden tests, so fix the
  root cause, not just the one assertion you see.)
- One root cause can make several tests fail. Some tickets may have no failing test until you look
  at the code path they describe.
- **AI assistant:** open a terminal in this folder and run `claude`. It follows `CLAUDE.md`: it can
  point you to the files and functions involved in a flow and answer any syntax or library question.
  It will not write code, confirm your guesses, or tell you what the bug is.
- Work the tickets in any order. They're listed roughly easiest to hardest.

## Setup

Requires Node 18+. No MongoDB install is needed: an in-memory MongoDB is started automatically.

```bash
npm run setup     # installs root, server and client dependencies
npm test          # runs server (Jest) + client (Vitest) suites
npm run dev       # starts BOTH: web UI http://localhost:5173  +  API http://localhost:5000
```

**Open the UI at http://localhost:5173.** Port 5000 is the JSON API only.
Run `npm run dev` from this root folder. Running it inside `server/` starts only the API.

The first run downloads a MongoDB binary (about 600 MB, one time only). The API isn't ready until
you see `[api] listening`.

Handy commands:

```bash
cd server && npx jest tests/orders.test.js           # one file
cd server && npx jest -t "cancel"                     # tests whose name matches
cd client && npx vitest                               # watch mode
```

Demo accounts (dev server): `alice@shoplite.dev / alice123` (customer), `admin@shoplite.dev / admin123` (admin).

Tip: run `git init && git add -A && git commit -m baseline` before you start, so you can `git diff` your work
or reset and retry later.

## Architecture

```
server/src
  app.js                 Express app: middleware + route mounting
  server.js              Boots DB (+ seed) and starts listening
  config/                env config, Mongo connection
  models/                User, Product, Order (Mongoose)
  middleware/            auth (protect / requireRole), asyncHandler, errorHandler
  routes/                /api/auth, /api/products, /api/orders, /api/analytics
  controllers/           HTTP handlers
  services/              orderService (checkout), analyticsService, pricing
  utils/ cache/          LRUCache, shared product cache, JWT signing, AppError
server/tests             Jest + Supertest + mongodb-memory-server

client/src
  App.jsx                Tabs: products / cart / orders / login / top sellers
  api.js                 fetch wrapper (adds Bearer token)
  state/                 cartReducer (+ tests), CartContext (useReducer)
  components/            ProductList, Cart, Orders, Login, TopSellers
```

## Tickets

| ID | Area | Report |
|----|------|--------|
| SL-101 | Catalog | Customers browsing the catalog page by page never see some products, even though the inventory `total` says they exist. |
| SL-102 | Search | The storefront search bar shows an error no matter what you type. |
| SL-103 | Orders | The warehouse sees orders flip to **cancelled** after they shipped, even though the customer saw an error when they clicked Cancel. |
| SL-104 | Orders | Customers get "You do not have access to this order" when they open **their own** order. Admins can open it fine. |
| SL-105 | Security | Pen-test finding (severity HIGH): the API trusts tokens it should reject. |
| SL-106 | Cart (web) | In the local dev build, clicking **Add to cart** on an item that's already in the cart adds 2. Some engineers can't reproduce it in the production build. |
| SL-107 | Analytics | The admin **Top sellers** ranking looks random. |
| SL-108 | Checkout | An order rejected for low stock still changes inventory for the other items in the cart. Ordering a product that doesn't exist returns 500. |
| SL-109 | Platform | The platform team wants to reuse `LRUCache` elsewhere, but it doesn't always evict the least recently used entry. |
| SL-110 | Checkout | In the last flash sale we sold more units than we had in stock. Nothing useful in the logs. |

## Done when

- `npm test` prints `server: PASS   client: PASS`
- `npm run dev` works: browse, search, add to cart, check out, view and cancel an order, and (as admin) see top sellers.

Answer key: `../_answer_keys/oa-01-shoplite.md.b64`. It's base64-encoded so you can't spoil it by accident. Decode it (PowerShell):

```powershell
[Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((Get-Content -Raw ..\_answer_keys\oa-01-shoplite.md.b64)))
```
