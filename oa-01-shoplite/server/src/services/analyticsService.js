const { roundCurrency } = require('./pricing');

/**
 * Returns the top-k best-selling products across all non-cancelled orders.
 *
 * Ranking: unitsSold descending; ties broken by product name ascending.
 * Each entry: { productId, name, unitsSold, revenue }
 */
function topSellingProducts(orders, k) {
  const tally = new Map();

  for (const order of orders) {
    if (order.status === 'cancelled') continue;

    for (const item of order.items) {
      const id = String(item.product);
      const entry = tally.get(id) || { productId: id, name: item.name, unitsSold: 0, revenue: 0 };
      entry.unitsSold += item.quantity;
      entry.revenue = roundCurrency(entry.revenue + item.price * item.quantity);
      tally.set(id, entry);
    }
  }

  return [...tally.values()]
    .sort((a, b) => b.unitsSold - a.unitsSold || a.name.localeCompare(b.name))
    .slice(0, k);
}

module.exports = { topSellingProducts };
