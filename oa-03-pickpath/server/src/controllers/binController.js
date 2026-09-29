const Bin = require('../models/Bin');
const AppError = require('../utils/AppError');
const asyncHandler = require('../middleware/asyncHandler');

// GET /api/bins?aisle=3
const listBins = asyncHandler(async (req, res) => {
  const filter = {};
  if (req.query.aisle) filter.aisle = Number(req.query.aisle);
  res.json(await Bin.find(filter).sort({ code: 1 }).lean());
});

// PATCH /api/bins/:id   { quantity }   (lead only: cycle-count correction)
const updateBin = asyncHandler(async (req, res) => {
  const { quantity } = req.body || {};
  if (!Number.isInteger(quantity) || quantity < 0) {
    throw new AppError(400, 'quantity must be a non-negative integer');
  }

  const bin = await Bin.findByIdAndUpdate(req.params.id, { quantity }, { runValidators: true });
  if (!bin) throw new AppError(404, 'Bin not found');

  res.json(bin);
});

module.exports = { listBins, updateBin };
