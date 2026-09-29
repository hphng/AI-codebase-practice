# ShopLite: AI-Assisted Coding Assessment (MERN)

**Practice set #1 · Difficulty: Medium · Suggested time: 90 minutes**

## Scenario

You just joined the team that owns **ShopLite**, a small MERN storefront (MongoDB, Express, React, Node).
There are two parts to this assessment:

- **Part A: build a feature.** Implement the "Frequently bought together" algorithm.
- **Part B: fix 5 bugs.** The previous on-call engineer left 5 tickets. Each has **one root cause**.

Do them in any order. When you're done, the whole test suite passes and the app works end to end.

## Rules

- **Don't edit the tests.** They are the spec. (The real assessment also runs hidden tests, so fix the
  root cause, not just the one assertion you see.)
- **AI assistant:** open a terminal in this folder and run `claude`. It follows `CLAUDE.md`. It can
  point you to files and functions, answer syntax/library questions, and explain a data structure or
  algorithm *you name*. It will not write code, design your approach, confirm your guesses, or tell you
  what a bug is.

## Setup

Requires Node 18+. No MongoDB install is needed: an in-memory MongoDB is started automatically.

```bash
npm run setup     # installs root, server and client dependencies
npm test          # runs server (Jest) + client (Vitest) suites
npm run dev       # starts BOTH: web UI http://localhost:5173  +  API http://localhost:5000
```

**Open the UI at http://localhost:5173.** Port 5000 is the JSON API only.
The first run downloads a MongoDB binary (about 600 MB, one time only).

Handy commands:

```bash
cd server && npx jest tests/recommendations.test.js   # one file
cd server && npx jest -t "cancel"                     # tests whose name matches
cd client && npx vitest                               # watch mode
```

Demo accounts (dev server): `alice@shoplite.dev / alice123` (customer), `admin@shoplite.dev / admin123` (admin).

## Part A: Frequently bought together

When a customer has items in their cart, the cart page shows **"Frequently bought with …"** suggestions.
The API endpoint `GET /api/products/:id/also-bought?k=3` and the React component already exist. The ranking
algorithm does not: `frequentlyBoughtTogether` in `server/src/services/recommendationService.js`
currently throws `501 Not Implemented`, so the suggestions box stays hidden.

Implement it. The full spec (inputs, output shape, ranking rules, performance requirement) is in the
function's doc comment. The examples are in `server/tests/recommendations.test.js`.

## Part B: Tickets

| ID | Area | Report |
|----|------|--------|
| SL-1 | Catalog | Customers browsing the catalog page by page never see some products, even though the inventory `total` says they exist. |
| SL-2 | Orders | The warehouse sees orders flip to **cancelled** after they shipped, even though the customer saw an error when they clicked Cancel. |
| SL-3 | Security | Pen-test finding (severity HIGH): the API trusts tokens it should reject. |
| SL-4 | Cart (web) | In the local dev build, clicking **Add to cart** on an item that's already in the cart adds 2. Some engineers can't reproduce it in the production build. |
| SL-5 | Checkout | In the last flash sale we sold more units than we had in stock. Nothing useful in the logs. |

## Architecture

```
server/src
  app.js                 Express app: middleware + route mounting
  models/                User, Product, Order (Mongoose)
  middleware/            auth (protect / requireRole), asyncHandler, errorHandler
  routes/ controllers/   /api/auth, /api/products, /api/orders, /api/analytics
  services/              orderService (checkout), recommendationService (Part A), analyticsService, pricing
  utils/ cache/          LRUCache, shared product cache, JWT signing, AppError
server/tests             Jest + Supertest + mongodb-memory-server

client/src
  App.jsx                Tabs: products / cart / orders / login / top sellers
  state/                 cartReducer (+ tests), CartContext (useReducer)
  components/            ProductList, Cart, AlsoBought, Orders, Login, TopSellers
```

## Done when

- `npm test` prints `server: PASS   client: PASS`.
- `npm run dev` works end to end: browse, search, add to cart (with "Frequently bought with" suggestions),
  check out, view and cancel an order.

Answer key (bugs + a reference solution for Part A): `../_answer_keys/01-shoplite.md.b64`. It's local only
and base64-encoded so you can't spoil it by accident. Decode it (PowerShell):

```powershell
[Text.Encoding]::UTF8.GetString([Convert]::FromBase64String((Get-Content -Raw ..\_answer_keys\01-shoplite.md.b64)))
```
