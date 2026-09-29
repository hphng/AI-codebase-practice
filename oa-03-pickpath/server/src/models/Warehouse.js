const mongoose = require('mongoose');

// Floor plan. Each string is one row. 'S' = dock (start/end of every route),
// '.' = walkable floor, '#' = shelving (not walkable).
const warehouseSchema = new mongoose.Schema({
  name: { type: String, required: true },
  grid: {
    type: [String],
    required: true,
    validate: {
      validator: (rows) => rows.length > 0 && rows.every((r) => r.length === rows[0].length),
      message: 'grid rows must all have the same length',
    },
  },
});

module.exports = mongoose.model('Warehouse', warehouseSchema);
