const Picker = require('../models/Picker');
const AppError = require('../utils/AppError');
const asyncHandler = require('../middleware/asyncHandler');
const { signToken } = require('../utils/token');

const PIN_FORMAT = /^\d{4,8}$/;

// POST /api/auth/login   { badgeCode, pin }   (browser login)
const login = asyncHandler(async (req, res) => {
  const { badgeCode, pin } = req.body || {};
  if (typeof badgeCode !== 'string' || typeof pin !== 'string') {
    throw new AppError(400, 'badgeCode and pin are required');
  }

  const picker = await Picker.findOne({ badgeCode, active: true }).select('+pin');
  if (!picker || !(await picker.comparePin(pin))) throw new AppError(401, 'Invalid badge or PIN');

  res.json({ token: signToken(picker), picker: picker.toPublic() });
});

// POST /api/auth/badge   { badgeCode }   (trusted floor kiosks: scanning a badge is enough)
const badgeLogin = asyncHandler(async (req, res) => {
  const { badgeCode } = req.body || {};
  if (!badgeCode) throw new AppError(400, 'badgeCode is required');

  const picker = await Picker.findOne({ badgeCode, active: true });
  if (!picker) throw new AppError(401, 'Unknown badge');

  res.json({ token: signToken(picker), picker: picker.toPublic() });
});

// GET /api/auth/me
const me = asyncHandler(async (req, res) => {
  const picker = await Picker.findById(req.user.id);
  if (!picker) throw new AppError(401, 'Account no longer exists');
  res.json(picker.toPublic());
});

// PATCH /api/auth/pin   { currentPin, newPin }
const changePin = asyncHandler(async (req, res) => {
  const { currentPin, newPin } = req.body || {};
  if (typeof newPin !== 'string' || !PIN_FORMAT.test(newPin)) {
    throw new AppError(400, 'newPin must be 4-8 digits');
  }

  const picker = await Picker.findById(req.user.id).select('+pin');
  if (!picker || !(await picker.comparePin(currentPin))) throw new AppError(401, 'Current PIN is incorrect');

  await Picker.findByIdAndUpdate(picker._id, { pin: newPin });
  res.json({ message: 'PIN updated' });
});

module.exports = { login, badgeLogin, me, changePin };
