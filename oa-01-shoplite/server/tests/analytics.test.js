const request = require('supertest');
const app = require('../src/app');
const Order = require('../src/models/Order');
const { topSellingProducts } = require('../src/services/analyticsService');
const { createUser, createProduct } = require('./helpers');

const item = (product, name, price, quantity) => ({ product, name, price, quantity });

const ORDERS = [
  { status: 'paid', items: [item('p1', 'Kindle', 100, 1), item('p2', 'Echo Dot', 49.99, 3)] },
  { status: 'delivered', items: [item('p3', 'Fire TV', 40, 3), item('p1', 'Kindle', 100, 1)] },
  { status: 'cancelled', items: [item('p4', 'Ring Doorbell', 200, 10)] },
  { status: 'pending', items: [item('p5', 'Blink Camera', 30, 5)] },
];

describe('topSellingProducts', () => {
  it('ranks by units sold (desc), breaking ties by name (asc)', () => {
    const top = topSellingProducts(ORDERS, 3);

    expect(top.map((p) => [p.name, p.unitsSold])).toEqual([
      ['Blink Camera', 5],
      ['Echo Dot', 3],
      ['Fire TV', 3],
    ]);
  });

  it('ignores cancelled orders', () => {
    const top = topSellingProducts(ORDERS, 10);
    expect(top.find((p) => p.name === 'Ring Doorbell')).toBeUndefined();
  });

  it('aggregates revenue per product', () => {
    const top = topSellingProducts(ORDERS, 10);
    const echo = top.find((p) => p.productId === 'p2');
    const kindle = top.find((p) => p.productId === 'p1');

    expect(echo.revenue).toBe(149.97);
    expect(kindle).toMatchObject({ unitsSold: 2, revenue: 200 });
  });

  it('returns everything when k exceeds the number of products', () => {
    expect(topSellingProducts(ORDERS, 50)).toHaveLength(4);
  });

  it('returns an empty list when there are no orders', () => {
    expect(topSellingProducts([], 5)).toEqual([]);
  });
});

describe('GET /api/analytics/top-products', () => {
  it('returns the ranking to admins', async () => {
    const { user, auth } = await createUser({ role: 'admin' });
    const a = await createProduct({ name: 'Alpha' });
    const b = await createProduct({ name: 'Bravo' });
    const c = await createProduct({ name: 'Charlie' });
    await Order.insertMany([
      { user: user._id, total: 10, items: [item(a._id, 'Alpha', 10, 1)] },
      { user: user._id, total: 40, items: [item(b._id, 'Bravo', 10, 4)] },
      { user: user._id, total: 20, items: [item(c._id, 'Charlie', 10, 2)] },
      { user: user._id, total: 50, status: 'cancelled', items: [item(a._id, 'Alpha', 10, 5)] },
    ]);

    const res = await request(app).get('/api/analytics/top-products?k=2').set(auth);

    expect(res.status).toBe(200);
    expect(res.body.map((p) => p.name)).toEqual(['Bravo', 'Charlie']);
  });

  it('is admin-only', async () => {
    const { auth } = await createUser();
    const res = await request(app).get('/api/analytics/top-products').set(auth);
    expect(res.status).toBe(403);
  });
});
