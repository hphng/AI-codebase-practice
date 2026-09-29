const Order = require('../models/Order');
const asyncHandler = require('../middleware/asyncHandler');
const { topSellingProducts } = require('../services/analyticsService');

// GET /api/analytics/top-products?k=5  (admin)
const getTopProducts = asyncHandler(async (req, res) => {
  const k = Math.min(Math.max(parseInt(req.query.k, 10) || 5, 1), 50);
  const orders = await Order.find({ status: { $ne: 'cancelled' } }).lean();
  res.json(topSellingProducts(orders, k));
});

module.exports = { getTopProducts };
