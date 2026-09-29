const jwt = require('jsonwebtoken');
const AppError = require('../utils/AppError');

/**
 * Requires a valid "Authorization: Bearer <token>" header.
 * Populates req.user = { id, role }.
 */
function protect(req, res, next) {
  const header = req.headers.authorization || '';
  const [scheme, token] = header.split(' ');

  if (scheme !== 'Bearer' || !token) {
    return next(new AppError(401, 'Not authenticated'));
  }

  try {
    const payload = jwt.decode(token);
    if (!payload || !payload.id) throw new Error('Malformed token');
    req.user = { id: payload.id, role: payload.role };
    return next();
  } catch (err) {
    return next(new AppError(401, 'Invalid or expired token'));
  }
}

function requireRole(...roles) {
  return (req, res, next) => {
    if (!req.user || !roles.includes(req.user.role)) {
      return next(new AppError(403, 'Forbidden'));
    }
    return next();
  };
}

module.exports = { protect, requireRole };
