const Bin = require('../models/Bin');
const PickList = require('../models/PickList');
const Warehouse = require('../models/Warehouse');
const AppError = require('../utils/AppError');
const asyncHandler = require('../middleware/asyncHandler');
const { assertFitsInTote } = require('../services/toteService');
const { planPickRoute } = require('../services/routePlanner');

// GET /api/picklists
const listPickLists = asyncHandler(async (req, res) => {
  res.json(await PickList.find().sort({ createdAt: -1 }).lean());
});

// POST /api/picklists   { name, toteMaxKg, items: [{ sku, quantity }] }   (lead only)
const createPickList = asyncHandler(async (req, res) => {
  const { name, toteMaxKg, items } = req.body || {};
  if (!name || typeof toteMaxKg !== 'number' || toteMaxKg <= 0) {
    throw new AppError(400, 'name and a positive toteMaxKg are required');
  }
  if (!Array.isArray(items) || items.length === 0) throw new AppError(400, 'items must be a non-empty array');
  for (const item of items) {
    if (!item || typeof item.sku !== 'string' || !Number.isInteger(item.quantity) || item.quantity < 1) {
      throw new AppError(400, 'each item needs a sku and a positive integer quantity');
    }
  }

  const bins = await Bin.find({ sku: { $in: items.map((i) => i.sku) } }).lean();
  const binsBySku = new Map(bins.map((b) => [b.sku, b]));
  const missing = items.filter((i) => !binsBySku.has(i.sku)).map((i) => i.sku);
  if (missing.length > 0) throw new AppError(400, `Unknown sku: ${missing.join(', ')}`);

  const totalKg = assertFitsInTote(items, binsBySku, toteMaxKg);
  const pickList = await PickList.create({ name, toteMaxKg, items, totalKg, createdBy: req.user.id });
  res.status(201).json(pickList);
});

// POST /api/picklists/:id/route
const planRoute = asyncHandler(async (req, res) => {
  const pickList = await PickList.findById(req.params.id).lean();
  if (!pickList) throw new AppError(404, 'Pick list not found');

  const warehouse = await Warehouse.findOne().lean();
  if (!warehouse) throw new AppError(500, 'No warehouse configured');

  const bins = await Bin.find({ sku: { $in: pickList.items.map((i) => i.sku) } }).lean();
  const stops = bins.map((b) => ({ sku: b.sku, row: b.row, col: b.col }));

  res.json({ pickListId: String(pickList._id), grid: warehouse.grid, stops, ...planPickRoute(warehouse.grid, stops) });
});

module.exports = { listPickLists, createPickList, planRoute };
