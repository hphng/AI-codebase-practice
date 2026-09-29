const express = require('express');
const cors = require('cors');
const { authRouter, binRouter, pickListRouter, warehouseRouter } = require('./routes');
const { notFound, errorHandler } = require('./middleware/errorHandler');

const app = express();

app.use(cors());
app.use(express.json());

app.get('/', (req, res) =>
  res.type('html').send('PickPath API is running. The web UI is at <a href="http://localhost:5174">http://localhost:5174</a>.')
);
app.get('/api/health', (req, res) => res.json({ status: 'ok' }));

app.use('/api/auth', authRouter);
app.use('/api/bins', binRouter);
app.use('/api/picklists', pickListRouter);
app.use('/api/warehouse', warehouseRouter);

app.use(notFound);
app.use(errorHandler);

module.exports = app;
