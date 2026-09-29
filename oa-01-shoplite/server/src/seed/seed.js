const User = require('../models/User');
const Product = require('../models/Product');
const Order = require('../models/Order');

const PRODUCTS = [
  ['Air Fryer', 'kitchen', 89.99, 12],
  ['Bluetooth Speaker', 'electronics', 39.5, 30],
  ['Board Game: Catan', 'toys', 44.0, 8],
  ['Cast Iron Skillet', 'kitchen', 29.95, 20],
  ['Ceramic Mug Set', 'kitchen', 24.0, 3],
  ['Desk Lamp', 'home', 19.99, 15],
  ['Drip Coffee Maker', 'kitchen', 49.99, 10],
  ['E-Reader', 'electronics', 129.99, 6],
  ['Floor Lamp', 'home', 64.0, 4],
  ['Hardcover: Clean Code', 'books', 34.99, 25],
  ['Hardcover: DDIA', 'books', 42.5, 18],
  ['Jigsaw Puzzle 1000pc', 'toys', 17.25, 22],
  ['Mechanical Keyboard', 'electronics', 99.0, 9],
  ['Memory Foam Pillow', 'home', 32.0, 14],
  ['Noise-Cancelling Headphones', 'electronics', 199.99, 5],
  ['Paperback: The Pragmatic Programmer', 'books', 27.99, 16],
  ['Plush Dinosaur', 'toys', 14.99, 40],
  ['Robot Vacuum', 'home', 249.0, 2],
  ['Smart Plug (2-pack)', 'electronics', 21.99, 50],
  ['Stainless Water Bottle', 'kitchen', 18.5, 35],
  ['Standing Desk Mat', 'home', 45.0, 7],
  ['USB-C Charger 65W', 'electronics', 35.99, 28],
  ['Wireless Mouse', 'electronics', 25.0, 33],
  ['Wooden Building Blocks', 'toys', 29.0, 11],
  ['Yoga Mat', 'home', 22.0, 19],
];

async function seed() {
  await Promise.all([User.deleteMany({}), Product.deleteMany({}), Order.deleteMany({})]);

  const [admin, alice] = await User.create([
    { name: 'Admin', email: 'admin@shoplite.dev', password: 'admin123', role: 'admin' },
    { name: 'Alice', email: 'alice@shoplite.dev', password: 'alice123' },
    { name: 'Bob', email: 'bob@shoplite.dev', password: 'bob12345' },
  ]);

  const products = await Product.insertMany(
    PRODUCTS.map(([name, category, price, stock]) => ({
      name,
      category,
      price,
      stock,
      description: `${name} - a ShopLite bestseller.`,
    }))
  );
  const byName = Object.fromEntries(products.map((p) => [p.name, p]));

  const line = (name, quantity) => ({
    product: byName[name]._id,
    name,
    price: byName[name].price,
    quantity,
  });
  const orderOf = (user, status, lines) => ({
    user: user._id,
    status,
    items: lines,
    total: Math.round(lines.reduce((s, l) => s + l.price * l.quantity, 0) * 100) / 100,
  });

  await Order.create([
    orderOf(alice, 'delivered', [line('Desk Lamp', 2), line('Wireless Mouse', 1)]),
    orderOf(alice, 'shipped', [line('Smart Plug (2-pack)', 4)]),
    orderOf(alice, 'pending', [line('Plush Dinosaur', 3), line('Desk Lamp', 1)]),
    orderOf(admin, 'paid', [line('E-Reader', 1), line('Wireless Mouse', 2)]),
  ]);

  console.log('[seed] users: admin@shoplite.dev / admin123, alice@shoplite.dev / alice123, bob@shoplite.dev / bob12345');
}

module.exports = { seed };
