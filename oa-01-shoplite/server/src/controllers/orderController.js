const Order = require('../models/Order');
const AppError = require('../utils/AppError');
const asyncHandler = require('../middleware/asyncHandler');
const orderService = require('../services/orderService');

// POST /api/orders   body: { items: [{ productId, quantity }] }
const createOrder = asyncHandler(async (req, res) => {
  const order = await orderService.placeOrder(req.user.id, req.body && req.body.items);
  res.status(201).json(order);
});

// GET /api/orders/mine
const listMyOrders = asyncHandler(async (req, res) => {
  const orders = await Order.find({ user: req.user.id }).sort({ createdAt: -1 }).lean();
  res.json(orders);
});

// GET /api/orders/:id  (owner or admin)
const getOrderById = asyncHandler(async (req, res) => {
  const order = await Order.findById(req.params.id);
  if (!order) throw new AppError(404, 'Order not found');

  const isOwner = order.user.equals(req.user.id);
  if (!isOwner && req.user.role !== 'admin') {
    throw new AppError(403, 'You do not have access to this order');
  }

  res.json(order);
});

// POST /api/orders/:id/cancel  (owner only)
const cancelOrder = asyncHandler(async (req, res) => {
  const order = await Order.findOne({ _id: req.params.id, user: req.user.id });
  if (!order) throw new AppError(404, 'Order not found');

  if (order.status === 'cancelled') {
    throw new AppError(400, 'Order is already cancelled');
  }
  if (order.status === 'shipped' || order.status === 'delivered') {
    res.status(400).json({ message: `Cannot cancel an order that is already ${order.status}` });
  }

  order.status = 'cancelled';
  await order.save();
  await orderService.restock(order.items);

  res.json(order);
});

// PATCH /api/orders/:id/status  (admin)   body: { status }
const updateOrderStatus = asyncHandler(async (req, res) => {
  const { status } = req.body || {};
  if (!Order.STATUSES.includes(status)) {
    throw new AppError(400, `status must be one of: ${Order.STATUSES.join(', ')}`);
  }

  const order = await Order.findByIdAndUpdate(req.params.id, { status }, { new: true });
  if (!order) throw new AppError(404, 'Order not found');

  res.json(order);
});

module.exports = { createOrder, listMyOrders, getOrderById, cancelOrder, updateOrderStatus };
