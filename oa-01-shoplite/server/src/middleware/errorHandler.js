function notFound(req, res) {
  res.status(404).json({ message: `Route not found: ${req.method} ${req.originalUrl}` });
}

// eslint-disable-next-line no-unused-vars
function errorHandler(err, req, res, next) {
  if (res.headersSent) return next(err);

  if (err.name === 'CastError') {
    return res.status(400).json({ message: `Invalid ${err.path}: ${err.value}` });
  }
  if (err.name === 'ValidationError') {
    return res.status(400).json({ message: err.message });
  }
  if (err.code === 11000) {
    return res.status(409).json({ message: 'Duplicate key' });
  }

  const status = err.statusCode || 500;
  if (status >= 500 && process.env.NODE_ENV !== 'test') console.error(err);
  res.status(status).json({ message: status >= 500 ? 'Internal Server Error' : err.message });
}

module.exports = { notFound, errorHandler };
