const mongoose = require('mongoose');

// A storage bin holding one SKU. (row, col) is the pick face: the walkable grid cell
// in front of the shelf where a picker stands to pick from this bin.
const binSchema = new mongoose.Schema(
  {
    code: { type: String, required: true, unique: true },
    sku: { type: String, required: true, unique: true },
    productName: { type: String, required: true },
    aisle: { type: Number, required: true, min: 1 },
    row: { type: Number, required: true, min: 0 },
    col: { type: Number, required: true, min: 0 },
    quantity: { type: Number, required: true, min: 0 },
    weightKg: { type: Number, required: true, min: 0 },  // weight of ONE unit, up to 3 decimals
  },
  { timestamps: true }
);

module.exports = mongoose.model('Bin', binSchema);
