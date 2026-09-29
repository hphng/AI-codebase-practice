const express = require('express');
const {
  createOrder,
  listMyOrders,
  getOrderById,
  cancelOrder,
  updateOrderStatus,
} = require('../controllers/orderController');
const { protect, requireRole } = require('../middleware/auth');

const router = express.Router();

router.use(protect);

router.post('/', createOrder);
router.get('/mine', listMyOrders);
router.get('/:id', getOrderById);
router.post('/:id/cancel', cancelOrder);
router.patch('/:id/status', requireRole('admin'), updateOrderStatus);

module.exports = router;
