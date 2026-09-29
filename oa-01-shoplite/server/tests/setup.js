const mongoose = require('mongoose');
const { MongoMemoryServer } = require('mongodb-memory-server');
const productCache = require('../src/cache/productCache');

let mongod;

beforeAll(async () => {
  mongod = await MongoMemoryServer.create();
  await mongoose.connect(mongod.getUri());
  await Promise.all(Object.values(mongoose.models).map((m) => m.init()));
}, 120000);

afterEach(async () => {
  const collections = Object.values(mongoose.connection.collections);
  await Promise.all(collections.map((c) => c.deleteMany({})));
  productCache.clear();
});

afterAll(async () => {
  await mongoose.disconnect();
  if (mongod) await mongod.stop();
});
