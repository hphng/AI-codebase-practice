const mongoose = require('mongoose');

const STATUSES = ['pending', 'paid', 'shipped', 'delivered', 'cancelled'];

const orderItemSchema = new mongoose.Schema(
  {
    product: { type: mongoose.Schema.Types.ObjectId, ref: 'Product', required: true },
    name: { type: String, required: true },
    price: { type: Number, required: true, min: 0 },
    quantity: { type: Number, required: true, min: 1 },
  },
  { _id: false }
);

const orderSchema = new mongoose.Schema(
  {
    user: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true, index: true },
    items: { type: [orderItemSchema], required: true },
    total: { type: Number, required: true, min: 0 },
    status: { type: String, enum: STATUSES, default: 'pending' },
  },
  { timestamps: true }
);

const Order = mongoose.model('Order', orderSchema);
Order.STATUSES = STATUSES;

module.exports = Order;
