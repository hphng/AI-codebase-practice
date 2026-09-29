const app = require('./app');
const config = require('./config');
const { connectDB } = require('./config/db');
const { seed } = require('./seed/seed');

async function start() {
  const { usingMemory } = await connectDB();
  if (usingMemory || process.env.SEED === 'true') {
    await seed();
  }

  app.listen(config.port, () => {
    console.log(`[api] listening on http://localhost:${config.port}`);
  });
}

start().catch((err) => {
  console.error('[api] failed to start', err);
  process.exit(1);
});
