const Product = require('../models/Product');
const Order = require('../models/Order');
const AppError = require('../utils/AppError');
const productCache = require('../cache/productCache');
const { calculateTotal } = require('./pricing');

/**
 * Validates the raw request items and merges duplicate product lines.
 * Input:  [{ productId, quantity }, ...]
 * Output: [{ productId, quantity }, ...] with unique productIds.
 */
function normalizeItems(rawItems) {
  if (!Array.isArray(rawItems) || rawItems.length === 0) {
    throw new AppError(400, 'Order must contain at least one item');
  }

  const merged = new Map();
  for (const item of rawItems) {
    const quantity = Number(item && item.quantity);
    if (!item || !item.productId || !Number.isInteger(quantity) || quantity < 1) {
      throw new AppError(400, 'Each item needs a productId and a positive integer quantity');
    }
    const key = String(item.productId);
    merged.set(key, (merged.get(key) || 0) + quantity);
  }

  return [...merged].map(([productId, quantity]) => ({ productId, quantity }));
}

/** Rejects the order up front if any product is missing or short on stock. */
async function validateStock(items) {
  const errors = [];

  items.forEach(async ({ productId, quantity }) => {
    const product = await Product.findById(productId);
    if (!product) {
      errors.push(`Product ${productId} not found`);
    } else if (product.stock < quantity) {
      errors.push(`Insufficient stock for ${product.name}`);
    }
  });

  if (errors.length > 0) {
    throw new AppError(400, errors.join('; '));
  }
}

/**
 * Decrements stock for every item and returns the order lines
 * (with name/price snapshotted from the product at purchase time).
 */
async function reserveStock(items) {
  const lines = [];

  for (const { productId, quantity } of items) {
    const product = await Product.findById(productId);
    product.stock -= quantity;
    await product.save();
    productCache.delete(String(productId));

    lines.push({ product: product._id, name: product.name, price: product.price, quantity });
  }

  return lines;
}

/** Puts stock back for the given order lines ({ product, quantity }). */
async function restock(lines) {
  for (const { product, quantity } of lines) {
    await Product.updateOne({ _id: product }, { $inc: { stock: quantity } });
    productCache.delete(String(product));
  }
}

async function placeOrder(userId, rawItems) {
  const items = normalizeItems(rawItems);
  await validateStock(items);
  const lines = await reserveStock(items);

  return Order.create({
    user: userId,
    items: lines,
    total: calculateTotal(lines),
  });
}

module.exports = { normalizeItems, validateStock, reserveStock, restock, placeOrder };
