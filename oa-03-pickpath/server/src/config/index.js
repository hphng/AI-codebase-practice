module.exports = {
  port: Number(process.env.PORT) || 5001,
  mongoUri: process.env.MONGO_URI || '',
  jwtSecret: process.env.JWT_SECRET || 'pickpath-dev-secret',
  jwtExpiresIn: process.env.JWT_EXPIRES_IN || '8h',
};
