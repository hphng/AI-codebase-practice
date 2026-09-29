module.exports = {
  port: Number(process.env.PORT) || 5000,
  mongoUri: process.env.MONGO_URI || '',
  jwtSecret: process.env.JWT_SECRET || 'shoplite-dev-secret',
  jwtExpiresIn: process.env.JWT_EXPIRES_IN || '1h',
};
