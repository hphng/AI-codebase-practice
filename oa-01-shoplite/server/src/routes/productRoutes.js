const express = require('express');
const {
  listProducts,
  searchProducts,
  getProductById,
  createProduct,
  updateProduct,
} = require('../controllers/productController');
const { protect, requireRole } = require('../middleware/auth');

const router = express.Router();

router.get('/', listProducts);
router.get('/:id', getProductById);
router.get('/search', searchProducts);

router.post('/', protect, requireRole('admin'), createProduct);
router.patch('/:id', protect, requireRole('admin'), updateProduct);

module.exports = router;
