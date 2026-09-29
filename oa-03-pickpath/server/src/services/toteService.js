const AppError = require('../utils/AppError');

/**
 * Total weight (kg) of a pick list. `binsBySku` maps sku -> Bin.
 * Unit weights have at most 3 decimal places (gram precision).
 */
function totalWeightKg(items, binsBySku) {
  let total = 0;
  for (const { sku, quantity } of items) {
    total += binsBySku.get(sku).weightKg * quantity;
  }
  return total;
}

/**
 * Throws 400 if the items don't fit in the tote. A tote may be filled exactly to
 * its limit: 3 x 0.1 kg fits in a 0.3 kg tote.
 */
function assertFitsInTote(items, binsBySku, toteMaxKg) {
  const total = totalWeightKg(items, binsBySku);
  if (total > toteMaxKg) {
    throw new AppError(400, `Items weigh ${total.toFixed(3)} kg, tote limit is ${toteMaxKg} kg`);
  }
  return Math.round(total * 1000) / 1000;
}

module.exports = { totalWeightKg, assertFitsInTote };
