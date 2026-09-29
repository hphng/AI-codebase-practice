const jwt = require('jsonwebtoken');
const config = require('../config');

function signToken(picker) {
  return jwt.sign({ id: String(picker._id), role: picker.role }, config.jwtSecret, {
    expiresIn: config.jwtExpiresIn,
  });
}

module.exports = { signToken };
