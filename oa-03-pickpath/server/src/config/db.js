const mongoose = require('mongoose');
const config = require('./index');

/**
 * Connects to MongoDB. If MONGO_URI is not set, an in-memory MongoDB instance
 * is started so the app runs without any local database install.
 */
async function connectDB() {
  let uri = config.mongoUri;
  let usingMemory = false;

  if (!uri) {
    const { MongoMemoryServer } = require('mongodb-memory-server');
    const memoryServer = await MongoMemoryServer.create();
    uri = memoryServer.getUri();
    usingMemory = true;
    console.log('[db] MONGO_URI not set, using in-memory MongoDB');
  }

  await mongoose.connect(uri);
  console.log('[db] connected');
  return { usingMemory };
}

module.exports = { connectDB };
