const mongoose = require('mongoose');
const bcrypt = require('bcryptjs');

const pickerSchema = new mongoose.Schema(
  {
    name: { type: String, required: true, trim: true },
    // Printed on the physical badge; the floor kiosks log people in by scanning it.
    badgeCode: { type: String, required: true, unique: true, select: false },
    // 4-8 digit PIN for logging in from a browser. Stored as a bcrypt hash.
    pin: { type: String, required: true, select: false },
    role: { type: String, enum: ['picker', 'lead'], default: 'picker' },
    active: { type: Boolean, default: true },
  },
  { timestamps: true }
);

pickerSchema.pre('save', async function hashPin() {
  if (!this.isModified('pin')) return;
  this.pin = await bcrypt.hash(this.pin, 10);
});

pickerSchema.methods.comparePin = function comparePin(candidate) {
  return bcrypt.compare(String(candidate), this.pin);
};

pickerSchema.methods.toPublic = function toPublic() {
  return { id: String(this._id), name: this.name, role: this.role };
};

module.exports = mongoose.model('Picker', pickerSchema);
