const AppError = require('../utils/AppError');

/**
 * PART A: implement this function.
 *
 * "Frequently bought together": given the order history, return up to `k` products that were
 * most often bought in the same order as `productId`.
 *
 * @param {Array<{ status: string, items: Array<{ product: any, name: string, quantity: number }> }>} orders
 *        `items[].product` may be a string or a Mongoose ObjectId. Compare ids with String(id).
 * @param {string} productId  the product being viewed
 * @param {number} k          maximum number of results
 * @returns {Array<{ productId: string, name: string, score: number }>}
 *
 * Rules
 *  1. Orders with status "cancelled" are ignored.
 *  2. score(P) = number of orders that contain BOTH `productId` and P (P !== productId).
 *     An order counts once per product, even if that product appears on several lines.
 *  3. Only products with score >= 1 are returned. The product itself is never returned.
 *  4. Sort by score (high to low), then name (A to Z), then productId (A to Z).
 *  5. Return at most `k` entries. If k <= 0, return [].
 *  6. A product's name is the same on every line that references it.
 *
 * Performance
 *  The order history can hold 100,000+ orders. Your solution must be close to linear in the
 *  total number of order lines (plus sorting the results). There is a performance test.
 *
 * Examples: see tests/recommendations.test.js
 */
function frequentlyBoughtTogether(orders, productId, k) {
  throw new AppError(501, 'frequentlyBoughtTogether is not implemented yet');
}

module.exports = { frequentlyBoughtTogether };
