const mongoose = require('mongoose');
const config = require('./index');

let memoryServer = null;

/**
 * Connects to MongoDB. If MONGO_URI is not set, an in-memory MongoDB instance
 * is started so the app runs without any local database install.
 */
async function connectDB() {
  let uri = config.mongoUri;

  if (!uri) {
    const { MongoMemoryServer } = require('mongodb-memory-server');
    memoryServer = await MongoMemoryServer.create();
    uri = memoryServer.getUri();
    console.log('[db] MONGO_URI not set, using in-memory MongoDB');
  }

  await mongoose.connect(uri);
  console.log('[db] connected');
  return { usingMemory: Boolean(memoryServer) };
}

async function disconnectDB() {
  await mongoose.disconnect();
  if (memoryServer) await memoryServer.stop();
}

module.exports = { connectDB, disconnectDB };
