const mongoose = require('mongoose');

const pickItemSchema = new mongoose.Schema(
  {
    sku: { type: String, required: true },
    quantity: { type: Number, required: true, min: 1 },
  },
  { _id: false }
);

const pickListSchema = new mongoose.Schema(
  {
    name: { type: String, required: true, trim: true },
    items: { type: [pickItemSchema], required: true },
    toteMaxKg: { type: Number, required: true, min: 0 },
    totalKg: { type: Number, required: true, min: 0 },
    status: { type: String, enum: ['open', 'done'], default: 'open' },
    createdBy: { type: mongoose.Schema.Types.ObjectId, ref: 'Picker' },
  },
  { timestamps: true }
);

module.exports = mongoose.model('PickList', pickListSchema);
