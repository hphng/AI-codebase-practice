const Product = require('../models/Product');
const Order = require('../models/Order');
const AppError = require('../utils/AppError');
const asyncHandler = require('../middleware/asyncHandler');
const productCache = require('../cache/productCache');
const { frequentlyBoughtTogether } = require('../services/recommendationService');

const escapeRegex = (s) => s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');

// GET /api/products?page=1&limit=10&category=books
const listProducts = asyncHandler(async (req, res) => {
  const page = Math.max(parseInt(req.query.page, 10) || 1, 1);
  const limit = Math.min(Math.max(parseInt(req.query.limit, 10) || 10, 1), 100);

  const filter = {};
  if (req.query.category) filter.category = String(req.query.category);

  const [items, total] = await Promise.all([
    Product.find(filter).sort({ name: 1 }).skip(page * limit).limit(limit).lean(),
    Product.countDocuments(filter),
  ]);

  res.json({ items, page, limit, total, totalPages: Math.ceil(total / limit) });
});

// GET /api/products/search?q=lamp
const searchProducts = asyncHandler(async (req, res) => {
  const q = String(req.query.q || '').trim();
  if (!q) throw new AppError(400, 'Search query "q" is required');

  const items = await Product.find({ name: new RegExp(escapeRegex(q), 'i') })
    .sort({ name: 1 })
    .limit(20)
    .lean();

  res.json({ items });
});

// GET /api/products/:id
const getProductById = asyncHandler(async (req, res) => {
  const { id } = req.params;

  const cached = productCache.get(id);
  if (cached) return res.json(cached);

  const product = await Product.findById(id).lean();
  if (!product) throw new AppError(404, 'Product not found');

  productCache.put(id, product);
  res.json(product);
});

// GET /api/products/:id/also-bought?k=4
const getAlsoBought = asyncHandler(async (req, res) => {
  const k = Math.min(Math.max(parseInt(req.query.k, 10) || 4, 1), 20);

  const product = await Product.findById(req.params.id).lean();
  if (!product) throw new AppError(404, 'Product not found');

  const orders = await Order.find({ 'items.product': product._id }).lean();
  const ranked = frequentlyBoughtTogether(orders, String(product._id), k);

  // Attach current price/stock so the client can offer "Add to cart".
  const details = await Product.find({ _id: { $in: ranked.map((r) => r.productId) } }).lean();
  const byId = new Map(details.map((p) => [String(p._id), p]));
  res.json(
    ranked
      .filter((r) => byId.has(r.productId))
      .map((r) => ({ ...r, price: byId.get(r.productId).price, stock: byId.get(r.productId).stock }))
  );
});

// POST /api/products (admin)
const createProduct = asyncHandler(async (req, res) => {
  const { name, description, category, price, stock } = req.body || {};
  const product = await Product.create({ name, description, category, price, stock });
  res.status(201).json(product);
});

// PATCH /api/products/:id (admin)
const updateProduct = asyncHandler(async (req, res) => {
  const allowed = ['name', 'description', 'category', 'price', 'stock'];
  const update = {};
  for (const field of allowed) {
    if (req.body[field] !== undefined) update[field] = req.body[field];
  }

  const product = await Product.findByIdAndUpdate(req.params.id, update, {
    new: true,
    runValidators: true,
  });
  if (!product) throw new AppError(404, 'Product not found');

  productCache.delete(String(product._id));
  res.json(product);
});

module.exports = {
  listProducts,
  searchProducts,
  getProductById,
  getAlsoBought,
  createProduct,
  updateProduct,
};
