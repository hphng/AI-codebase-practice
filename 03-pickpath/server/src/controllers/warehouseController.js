const Warehouse = require('../models/Warehouse');
const AppError = require('../utils/AppError');
const asyncHandler = require('../middleware/asyncHandler');

// GET /api/warehouse
const getWarehouse = asyncHandler(async (req, res) => {
  const warehouse = await Warehouse.findOne().lean();
  if (!warehouse) throw new AppError(404, 'No warehouse configured');
  res.json(warehouse);
});

module.exports = { getWarehouse };
