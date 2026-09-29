const request = require('supertest');
const app = require('../src/app');
const PickList = require('../src/models/PickList');
const Warehouse = require('../src/models/Warehouse');
const { assertFitsInTote } = require('../src/services/toteService');
const { createPicker, createBin } = require('./helpers');

const bins = (list) => new Map(list.map((b) => [b.sku, b]));

describe('assertFitsInTote', () => {
  it('accepts a tote filled exactly to its limit', () => {
    const b = bins([{ sku: 'A', weightKg: 0.1 }, { sku: 'B', weightKg: 0.7 }, { sku: 'C', weightKg: 0.2 }]);
    expect(assertFitsInTote([{ sku: 'A', quantity: 3 }], b, 0.3)).toBe(0.3);
    expect(assertFitsInTote([{ sku: 'A', quantity: 1 }, { sku: 'C', quantity: 1 }], b, 0.3)).toBe(0.3);
    expect(assertFitsInTote([{ sku: 'B', quantity: 1 }, { sku: 'A', quantity: 1 }, { sku: 'C', quantity: 1 }], b, 1)).toBe(1);
  });

  it('rejects anything over the limit, even by one gram', () => {
    const b = bins([{ sku: 'A', weightKg: 0.1 }, { sku: 'G', weightKg: 0.001 }]);
    expect(() => assertFitsInTote([{ sku: 'A', quantity: 3 }, { sku: 'G', quantity: 1 }], b, 0.3)).toThrow(/tote/);
  });

  it('returns the total rounded to the gram', () => {
    const b = bins([{ sku: 'A', weightKg: 0.333 }]);
    expect(assertFitsInTote([{ sku: 'A', quantity: 3 }], b, 5)).toBe(0.999);
  });
});

describe('POST /api/picklists', () => {
  let lead;
  beforeEach(async () => {
    lead = await createPicker({ role: 'lead' });
    await createBin({ sku: 'CABLE', weightKg: 0.1 });
    await createBin({ sku: 'LAMP', weightKg: 1.25 });
  });

  it('creates a pick list and records its weight', async () => {
    const res = await request(app)
      .post('/api/picklists')
      .set(lead.auth)
      .send({ name: 'Wave', toteMaxKg: 5, items: [{ sku: 'CABLE', quantity: 2 }, { sku: 'LAMP', quantity: 1 }] });

    expect(res.status).toBe(201);
    expect(res.body.totalKg).toBe(1.45);
    expect(await PickList.countDocuments()).toBe(1);
  });

  it('accepts a list that exactly fills the tote', async () => {
    const res = await request(app)
      .post('/api/picklists')
      .set(lead.auth)
      .send({ name: 'Exact', toteMaxKg: 0.3, items: [{ sku: 'CABLE', quantity: 3 }] });

    expect(res.status).toBe(201);
    expect(res.body.totalKg).toBe(0.3);
  });

  it('rejects a list heavier than the tote', async () => {
    const res = await request(app)
      .post('/api/picklists')
      .set(lead.auth)
      .send({ name: 'Heavy', toteMaxKg: 2, items: [{ sku: 'LAMP', quantity: 2 }] });

    expect(res.status).toBe(400);
    expect(await PickList.countDocuments()).toBe(0);
  });

  it('rejects unknown skus and bad items', async () => {
    const unknown = await request(app)
      .post('/api/picklists')
      .set(lead.auth)
      .send({ name: 'X', toteMaxKg: 5, items: [{ sku: 'NOPE', quantity: 1 }] });
    expect(unknown.status).toBe(400);

    const bad = await request(app)
      .post('/api/picklists')
      .set(lead.auth)
      .send({ name: 'X', toteMaxKg: 5, items: [{ sku: 'CABLE', quantity: 0 }] });
    expect(bad.status).toBe(400);
  });

  it('is lead-only', async () => {
    const { auth } = await createPicker();
    const res = await request(app)
      .post('/api/picklists')
      .set(auth)
      .send({ name: 'X', toteMaxKg: 5, items: [{ sku: 'CABLE', quantity: 1 }] });
    expect(res.status).toBe(403);
  });
});

describe('POST /api/picklists/:id/route', () => {
  it('plans the walking route for the pick list', async () => {
    const { auth } = await createPicker();
    await Warehouse.create({ name: 'T', grid: ['S.#..', '..#.#', '.....'] });
    await createBin({ sku: 'A', row: 0, col: 3 });
    await createBin({ sku: 'B', row: 2, col: 4 });
    await createBin({ sku: 'C', row: 1, col: 1 });
    const list = await PickList.create({
      name: 'Route me',
      toteMaxKg: 10,
      totalKg: 1.5,
      items: [{ sku: 'A', quantity: 1 }, { sku: 'B', quantity: 1 }, { sku: 'C', quantity: 1 }],
    });

    const res = await request(app).post(`/api/picklists/${list._id}/route`).set(auth);

    expect(res.status).toBe(200);
    expect(res.body).toMatchObject({ order: ['C', 'B', 'A'], totalSteps: 16, unreachable: [] });
    expect(res.body.grid).toHaveLength(3);
  });
});
