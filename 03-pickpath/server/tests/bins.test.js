const request = require('supertest');
const mongoose = require('mongoose');
const app = require('../src/app');
const Bin = require('../src/models/Bin');
const { createPicker, createBin } = require('./helpers');

describe('GET /api/bins', () => {
  it('lists bins sorted by code and filters by aisle', async () => {
    const { auth } = await createPicker();
    await createBin({ code: 'A02-1', aisle: 2 });
    await createBin({ code: 'A01-2', aisle: 1 });
    await createBin({ code: 'A01-1', aisle: 1 });

    const all = await request(app).get('/api/bins').set(auth);
    expect(all.status).toBe(200);
    expect(all.body.map((b) => b.code)).toEqual(['A01-1', 'A01-2', 'A02-1']);

    const aisle1 = await request(app).get('/api/bins?aisle=1').set(auth);
    expect(aisle1.body.map((b) => b.code)).toEqual(['A01-1', 'A01-2']);
  });

  it('requires authentication', async () => {
    const res = await request(app).get('/api/bins');
    expect(res.status).toBe(401);
  });
});

describe('PATCH /api/bins/:id', () => {
  it('returns the bin with the corrected quantity', async () => {
    const { auth } = await createPicker({ role: 'lead' });
    const bin = await createBin({ quantity: 12 });

    const res = await request(app).patch(`/api/bins/${bin._id}`).set(auth).send({ quantity: 7 });

    expect(res.status).toBe(200);
    expect(res.body.quantity).toBe(7);
    expect((await Bin.findById(bin._id)).quantity).toBe(7);
  });

  it('rejects invalid quantities', async () => {
    const { auth } = await createPicker({ role: 'lead' });
    const bin = await createBin();
    for (const quantity of [-1, 2.5, '3', null]) {
      const res = await request(app).patch(`/api/bins/${bin._id}`).set(auth).send({ quantity });
      expect(res.status).toBe(400);
    }
  });

  it('returns 404 for an unknown bin', async () => {
    const { auth } = await createPicker({ role: 'lead' });
    const res = await request(app).patch(`/api/bins/${new mongoose.Types.ObjectId()}`).set(auth).send({ quantity: 1 });
    expect(res.status).toBe(404);
  });

  it('is lead-only', async () => {
    const { auth } = await createPicker();
    const bin = await createBin();
    const res = await request(app).patch(`/api/bins/${bin._id}`).set(auth).send({ quantity: 1 });
    expect(res.status).toBe(403);
  });
});
