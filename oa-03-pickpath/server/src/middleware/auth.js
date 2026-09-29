const jwt = require('jsonwebtoken');
const config = require('../config');
const AppError = require('../utils/AppError');

/** Requires "Authorization: Bearer <token>". Populates req.user = { id, role }. */
function protect(req, res, next) {
  const [scheme, token] = (req.headers.authorization || '').split(' ');
  if (scheme !== 'Bearer' || !token) return next(new AppError(401, 'Not authenticated'));

  try {
    const payload = jwt.verify(token, config.jwtSecret);
    req.user = { id: payload.id, role: payload.role };
    return next();
  } catch (err) {
    return next(new AppError(401, 'Invalid or expired token'));
  }
}

function requireRole(...roles) {
  return (req, res, next) =>
    req.user && roles.includes(req.user.role) ? next() : next(new AppError(403, 'Forbidden'));
}

module.exports = { protect, requireRole };
