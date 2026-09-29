const User = require('../src/models/User');
const Product = require('../src/models/Product');
const { signToken } = require('../src/utils/token');

let counter = 0;

async function createUser({ role = 'customer', name } = {}) {
  counter += 1;
  const user = await User.create({
    name: name || `User ${counter}`,
    email: `user${counter}@test.dev`,
    password: 'password123',
    role,
  });
  return { user, token: signToken(user), auth: { Authorization: `Bearer ${signToken(user)}` } };
}

function createProduct(overrides = {}) {
  return Product.create({ name: 'Widget', price: 10, stock: 10, category: 'general', ...overrides });
}

const settle = (ms = 150) => new Promise((resolve) => setTimeout(resolve, ms));

module.exports = { createUser, createProduct, settle };
