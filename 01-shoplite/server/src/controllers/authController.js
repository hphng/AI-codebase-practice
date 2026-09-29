const User = require('../models/User');
const AppError = require('../utils/AppError');
const asyncHandler = require('../middleware/asyncHandler');
const { signToken } = require('../utils/token');

const register = asyncHandler(async (req, res) => {
  const { name, email, password } = req.body || {};
  if (!name || !email || !password) {
    throw new AppError(400, 'name, email and password are required');
  }
  if (String(password).length < 6) {
    throw new AppError(400, 'Password must be at least 6 characters');
  }

  const existing = await User.findOne({ email: String(email).toLowerCase() });
  if (existing) throw new AppError(409, 'Email is already registered');

  // role is never taken from the request body
  const user = await User.create({ name, email, password });
  res.status(201).json({ token: signToken(user), user: user.toPublic() });
});

const login = asyncHandler(async (req, res) => {
  const { email, password } = req.body || {};
  if (!email || !password) throw new AppError(400, 'email and password are required');

  const user = await User.findOne({ email: String(email).toLowerCase() }).select('+password');
  if (!user || !(await user.comparePassword(password))) {
    throw new AppError(401, 'Invalid email or password');
  }

  res.json({ token: signToken(user), user: user.toPublic() });
});

const me = asyncHandler(async (req, res) => {
  const user = await User.findById(req.user.id);
  if (!user) throw new AppError(401, 'User no longer exists');
  res.json(user.toPublic());
});

module.exports = { register, login, me };
