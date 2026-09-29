const LRUCache = require('../utils/lruCache');

// Shared cache for product detail lookups, keyed by product id (string).
module.exports = new LRUCache(100);
