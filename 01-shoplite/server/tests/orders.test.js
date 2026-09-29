const request = require('supertest');
const mongoose = require('mongoose');
const app = require('../src/app');
const Order = require('../src/models/Order');
const Product = require('../src/models/Product');
const { createUser, createProduct, settle } = require('./helpers');

let customer;
let otherCustomer;
let admin;
let lamp;
let mug;

beforeEach(async () => {
  customer = await createUser();
  otherCustomer = await createUser();
  admin = await createUser({ role: 'admin' });
  lamp = await createProduct({ name: 'Desk Lamp', price: 19.99, stock: 5 });
  mug = await createProduct({ name: 'Mug', price: 7.5, stock: 2 });
});

const placeOrder = (auth, items) => request(app).post('/api/orders').set(auth).send({ items });
const stockOf = async (product) => (await Product.findById(product._id)).stock;

describe('POST /api/orders', () => {
  it('places an order, snapshots prices and decrements stock', async () => {
    const res = await placeOrder(customer.auth, [
      { productId: lamp._id, quantity: 2 },
      { productId: mug._id, quantity: 1 },
    ]);

    expect(res.status).toBe(201);
    expect(res.body.status).toBe('pending');
    expect(res.body.total).toBe(47.48);
    expect(res.body.items).toHaveLength(2);
    expect(await stockOf(lamp)).toBe(3);
    expect(await stockOf(mug)).toBe(1);
  });

  it('merges duplicate lines for the same product', async () => {
    const res = await placeOrder(customer.auth, [
      { productId: lamp._id, quantity: 1 },
      { productId: lamp._id, quantity: 2 },
    ]);

    expect(res.status).toBe(201);
    expect(res.body.items).toEqual([expect.objectContaining({ name: 'Desk Lamp', quantity: 3 })]);
    expect(await stockOf(lamp)).toBe(2);
  });

  it('rejects an empty order', async () => {
    const res = await placeOrder(customer.auth, []);
    expect(res.status).toBe(400);
  });

  it('requires authentication', async () => {
    const res = await request(app).post('/api/orders').send({ items: [] });
    expect(res.status).toBe(401);
  });

  it('rejects an order for a product that does not exist', async () => {
    const res = await placeOrder(customer.auth, [
      { productId: new mongoose.Types.ObjectId(), quantity: 1 },
    ]);

    expect(res.status).toBe(400);
    expect(await Order.countDocuments()).toBe(0);
  });

  it('rejects insufficient stock without touching any inventory', async () => {
    const res = await placeOrder(customer.auth, [
      { productId: lamp._id, quantity: 1 },
      { productId: mug._id, quantity: 5 },
    ]);

    expect(res.status).toBe(400);
    expect(res.body.message).toMatch(/stock/i);
    expect(await stockOf(lamp)).toBe(5);
    expect(await stockOf(mug)).toBe(2);
    expect(await Order.countDocuments()).toBe(0);
  });

  it('never oversells under concurrent orders', async () => {
    const limited = await createProduct({ name: 'Flash Deal', price: 1, stock: 3 });

    const responses = await Promise.all(
      Array.from({ length: 6 }, () => placeOrder(customer.auth, [{ productId: limited._id, quantity: 1 }]))
    );

    const created = responses.filter((r) => r.status === 201);
    const rejected = responses.filter((r) => r.status !== 201);

    expect(created).toHaveLength(3);
    rejected.forEach((r) => expect([400, 409]).toContain(r.status));
    expect(await stockOf(limited)).toBe(0);
    expect(await Order.countDocuments()).toBe(3);
  });
});

describe('GET /api/orders', () => {
  let order;

  beforeEach(async () => {
    const res = await placeOrder(customer.auth, [{ productId: lamp._id, quantity: 1 }]);
    order = res.body;
  });

  it('lists only my orders', async () => {
    await placeOrder(otherCustomer.auth, [{ productId: mug._id, quantity: 1 }]);
    const res = await request(app).get('/api/orders/mine').set(customer.auth);

    expect(res.status).toBe(200);
    expect(res.body.map((o) => o._id)).toEqual([order._id]);
  });

  it('lets the owner view their order', async () => {
    const res = await request(app).get(`/api/orders/${order._id}`).set(customer.auth);

    expect(res.status).toBe(200);
    expect(res.body._id).toBe(order._id);
  });

  it("forbids viewing someone else's order", async () => {
    const res = await request(app).get(`/api/orders/${order._id}`).set(otherCustomer.auth);
    expect(res.status).toBe(403);
  });

  it('lets an admin view any order', async () => {
    const res = await request(app).get(`/api/orders/${order._id}`).set(admin.auth);
    expect(res.status).toBe(200);
  });

  it('returns 404 for an unknown order', async () => {
    const res = await request(app)
      .get(`/api/orders/${new mongoose.Types.ObjectId()}`)
      .set(customer.auth);
    expect(res.status).toBe(404);
  });
});

describe('POST /api/orders/:id/cancel', () => {
  let order;

  beforeEach(async () => {
    const res = await placeOrder(customer.auth, [{ productId: lamp._id, quantity: 2 }]);
    order = res.body;
  });

  it('cancels a pending order and restocks', async () => {
    const res = await request(app).post(`/api/orders/${order._id}/cancel`).set(customer.auth);

    expect(res.status).toBe(200);
    expect(res.body.status).toBe('cancelled');
    expect(await stockOf(lamp)).toBe(5);
  });

  it('refuses to cancel a shipped order and leaves it untouched', async () => {
    await request(app)
      .patch(`/api/orders/${order._id}/status`)
      .set(admin.auth)
      .send({ status: 'shipped' });

    const res = await request(app).post(`/api/orders/${order._id}/cancel`).set(customer.auth);
    expect(res.status).toBe(400);

    await settle(); // let any in-flight writes land before inspecting the DB
    expect((await Order.findById(order._id)).status).toBe('shipped');
    expect(await stockOf(lamp)).toBe(3);
  });

  it('refuses to cancel twice', async () => {
    await request(app).post(`/api/orders/${order._id}/cancel`).set(customer.auth);
    const res = await request(app).post(`/api/orders/${order._id}/cancel`).set(customer.auth);

    expect(res.status).toBe(400);
    expect(await stockOf(lamp)).toBe(5);
  });

  it("cannot cancel someone else's order", async () => {
    const res = await request(app).post(`/api/orders/${order._id}/cancel`).set(otherCustomer.auth);
    expect(res.status).toBe(404);
  });
});

describe('PATCH /api/orders/:id/status', () => {
  it('rejects unknown statuses', async () => {
    const created = await placeOrder(customer.auth, [{ productId: lamp._id, quantity: 1 }]);
    const res = await request(app)
      .patch(`/api/orders/${created.body._id}/status`)
      .set(admin.auth)
      .send({ status: 'teleported' });

    expect(res.status).toBe(400);
  });

  it('is admin-only', async () => {
    const created = await placeOrder(customer.auth, [{ productId: lamp._id, quantity: 1 }]);
    const res = await request(app)
      .patch(`/api/orders/${created.body._id}/status`)
      .set(customer.auth)
      .send({ status: 'delivered' });

    expect(res.status).toBe(403);
  });
});
