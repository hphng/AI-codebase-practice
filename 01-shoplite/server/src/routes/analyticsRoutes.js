const express = require('express');
const { getTopProducts } = require('../controllers/analyticsController');
const { protect, requireRole } = require('../middleware/auth');

const router = express.Router();

router.get('/top-products', protect, requireRole('admin'), getTopProducts);

module.exports = router;
