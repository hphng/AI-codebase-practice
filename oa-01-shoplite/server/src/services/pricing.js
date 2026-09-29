function roundCurrency(amount) {
  return Math.round(amount * 100) / 100;
}

/** Sums price * quantity over order lines, rounded to cents. */
function calculateTotal(lines) {
  return roundCurrency(lines.reduce((sum, line) => sum + line.price * line.quantity, 0));
}

module.exports = { roundCurrency, calculateTotal };
