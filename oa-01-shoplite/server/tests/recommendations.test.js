const request = require('supertest');
const mongoose = require('mongoose');
const app = require('../src/app');
const Order = require('../src/models/Order');
const { frequentlyBoughtTogether } = require('../src/services/recommendationService');
const { createUser, createProduct } = require('./helpers');

const line = (product, name, quantity = 1) => ({ product, name, price: 10, quantity });

const ORDERS = [
  { status: 'delivered', items: [line('kb', 'Keyboard'), line('mouse', 'Mouse'), line('pad', 'Mouse Pad')] },
  { status: 'paid', items: [line('kb', 'Keyboard'), line('mouse', 'Mouse')] },
  { status: 'pending', items: [line('kb', 'Keyboard'), line('cable', 'USB Cable'), line('cable', 'USB Cable', 2)] },
  { status: 'cancelled', items: [line('kb', 'Keyboard'), line('pad', 'Mouse Pad'), line('stand', 'Monitor Stand')] },
  { status: 'delivered', items: [line('mouse', 'Mouse'), line('pad', 'Mouse Pad')] },
  { status: 'delivered', items: [line('kb', 'Keyboard'), line('kb', 'Keyboard', 3), line('stand', 'Monitor Stand')] },
];

describe('frequentlyBoughtTogether', () => {
  it('ranks co-purchased products by number of shared orders, ties by name', () => {
    expect(frequentlyBoughtTogether(ORDERS, 'kb', 10)).toEqual([
      { productId: 'mouse', name: 'Mouse', score: 2 },
      { productId: 'stand', name: 'Monitor Stand', score: 1 },
      { productId: 'pad', name: 'Mouse Pad', score: 1 },
      { productId: 'cable', name: 'USB Cable', score: 1 },
    ]);
  });

  it('respects k', () => {
    expect(frequentlyBoughtTogether(ORDERS, 'kb', 2).map((r) => r.productId)).toEqual(['mouse', 'stand']);
  });

  it('returns [] when k <= 0', () => {
    expect(frequentlyBoughtTogether(ORDERS, 'kb', 0)).toEqual([]);
    expect(frequentlyBoughtTogether(ORDERS, 'kb', -3)).toEqual([]);
  });

  it('ignores cancelled orders', () => {
    const pad = frequentlyBoughtTogether(ORDERS, 'kb', 10).find((r) => r.productId === 'pad');
    expect(pad.score).toBe(1);
  });

  it('counts a product once per order even when it appears on several lines', () => {
    const cable = frequentlyBoughtTogether(ORDERS, 'kb', 10).find((r) => r.productId === 'cable');
    expect(cable.score).toBe(1);
  });

  it('never recommends the product itself', () => {
    expect(frequentlyBoughtTogether(ORDERS, 'kb', 10).some((r) => r.productId === 'kb')).toBe(false);
  });

  it('returns [] for a product nobody bought', () => {
    expect(frequentlyBoughtTogether(ORDERS, 'ghost', 5)).toEqual([]);
    expect(frequentlyBoughtTogether([], 'kb', 5)).toEqual([]);
  });

  it('breaks name ties by productId', () => {
    const orders = [{ status: 'paid', items: [line('x', 'X'), line('b2', 'Cable'), line('a9', 'Cable')] }];
    expect(frequentlyBoughtTogether(orders, 'x', 5).map((r) => r.productId)).toEqual(['a9', 'b2']);
  });

  it('accepts ObjectId product references', () => {
    const [a, b] = [new mongoose.Types.ObjectId(), new mongoose.Types.ObjectId()];
    const orders = [{ status: 'paid', items: [line(a, 'Alpha'), line(b, 'Beta')] }];
    expect(frequentlyBoughtTogether(orders, String(a), 5)).toEqual([{ productId: String(b), name: 'Beta', score: 1 }]);
  });

  it('handles a large order history quickly', () => {
    const orders = [];
    for (let i = 0; i < 60000; i++) {
      const items = [line('hero', 'Hero Product')];
      for (let j = 1; j <= 4; j++) {
        const id = `p${(i * 7 + j * 131) % 6000}`;
        items.push(line(id, `Product ${id}`));
      }
      orders.push({ status: 'delivered', items });
    }

    const started = Date.now();
    const top = frequentlyBoughtTogether(orders, 'hero', 5);
    const elapsed = Date.now() - started;

    expect(top).toHaveLength(5);
    expect(top[0].score).toBeGreaterThanOrEqual(top[4].score);
    expect(elapsed).toBeLessThan(1500);
  });
});

describe('GET /api/products/:id/also-bought', () => {
  it('returns ranked suggestions with price and stock', async () => {
    const { user } = await createUser();
    const kettle = await createProduct({ name: 'Kettle', price: 30 });
    const tea = await createProduct({ name: 'Green Tea', price: 5, stock: 7 });
    const mug = await createProduct({ name: 'Mug', price: 8 });
    const item = (p) => ({ product: p._id, name: p.name, price: p.price, quantity: 1 });

    await Order.insertMany([
      { user: user._id, total: 1, items: [item(kettle), item(tea)] },
      { user: user._id, total: 1, items: [item(kettle), item(tea), item(mug)] },
      { user: user._id, total: 1, items: [item(mug)] },
    ]);

    const res = await request(app).get(`/api/products/${kettle._id}/also-bought?k=5`);

    expect(res.status).toBe(200);
    expect(res.body).toEqual([
      { productId: String(tea._id), name: 'Green Tea', score: 2, price: 5, stock: 7 },
      { productId: String(mug._id), name: 'Mug', score: 1, price: 8, stock: 10 },
    ]);
  });

  it('returns 404 for an unknown product', async () => {
    const res = await request(app).get(`/api/products/${new mongoose.Types.ObjectId()}/also-bought`);
    expect(res.status).toBe(404);
  });
});
