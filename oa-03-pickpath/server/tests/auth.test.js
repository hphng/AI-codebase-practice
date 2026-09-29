const request = require('supertest');
const app = require('../src/app');
const Picker = require('../src/models/Picker');
const { createPicker } = require('./helpers');

describe('POST /api/auth/login (badge + PIN)', () => {
  beforeEach(() => createPicker({ badgeCode: 'PICK-7', pin: '2468' }));

  it('logs in with the right PIN', async () => {
    const res = await request(app).post('/api/auth/login').send({ badgeCode: 'PICK-7', pin: '2468' });
    expect(res.status).toBe(200);
    expect(res.body.token).toEqual(expect.any(String));
    expect(res.body.picker.role).toBe('picker');
  });

  it('rejects a wrong PIN', async () => {
    const res = await request(app).post('/api/auth/login').send({ badgeCode: 'PICK-7', pin: '0000' });
    expect(res.status).toBe(401);
  });

  it('rejects non-string credentials', async () => {
    const res = await request(app).post('/api/auth/login').send({ badgeCode: { $gt: '' }, pin: '2468' });
    expect(res.status).toBe(400);
  });
});

describe('POST /api/auth/badge (kiosk scan)', () => {
  beforeEach(async () => {
    await createPicker({ badgeCode: 'LEAD-1', role: 'lead' });
    await createPicker({ badgeCode: 'PICK-2' });
  });

  it('logs in with a valid badge', async () => {
    const res = await request(app).post('/api/auth/badge').send({ badgeCode: 'PICK-2' });
    expect(res.status).toBe(200);
    expect(res.body.picker.role).toBe('picker');
  });

  it('rejects an unknown badge', async () => {
    const res = await request(app).post('/api/auth/badge').send({ badgeCode: 'NOPE-9' });
    expect(res.status).toBe(401);
  });

  it('rejects a missing badge', async () => {
    const res = await request(app).post('/api/auth/badge').send({});
    expect(res.status).toBe(400);
  });

  it('only accepts a badge code string (no query operators)', async () => {
    for (const badgeCode of [{ $gt: '' }, { $ne: null }, { $regex: '.*' }, ['LEAD-1'], 12345]) {
      const res = await request(app).post('/api/auth/badge').send({ badgeCode });
      expect([400, 401]).toContain(res.status);
      expect(res.body.token).toBeUndefined();
    }
  });
});

describe('PATCH /api/auth/pin', () => {
  let auth;
  beforeEach(async () => {
    ({ auth } = await createPicker({ badgeCode: 'PICK-5', pin: '1357' }));
  });

  it('lets a picker log in with the new PIN (and not the old one)', async () => {
    const change = await request(app).patch('/api/auth/pin').set(auth).send({ currentPin: '1357', newPin: '97531' });
    expect(change.status).toBe(200);

    const withNew = await request(app).post('/api/auth/login').send({ badgeCode: 'PICK-5', pin: '97531' });
    expect(withNew.status).toBe(200);

    const withOld = await request(app).post('/api/auth/login').send({ badgeCode: 'PICK-5', pin: '1357' });
    expect(withOld.status).toBe(401);
  });

  it('never stores the PIN in plain text', async () => {
    await request(app).patch('/api/auth/pin').set(auth).send({ currentPin: '1357', newPin: '97531' });
    const stored = await Picker.findOne({ badgeCode: 'PICK-5' }).select('+pin').lean();
    expect(stored.pin).not.toBe('97531');
    expect(stored.pin).toMatch(/^\$2[aby]\$/);
  });

  it('requires the current PIN', async () => {
    const res = await request(app).patch('/api/auth/pin').set(auth).send({ currentPin: '0000', newPin: '97531' });
    expect(res.status).toBe(401);
  });

  it('validates the new PIN format', async () => {
    const res = await request(app).patch('/api/auth/pin').set(auth).send({ currentPin: '1357', newPin: '12' });
    expect(res.status).toBe(400);
  });
});

describe('GET /api/auth/me', () => {
  it('returns the current picker', async () => {
    const { picker, auth } = await createPicker();
    const res = await request(app).get('/api/auth/me').set(auth);
    expect(res.status).toBe(200);
    expect(res.body.id).toBe(String(picker._id));
  });

  it('requires a token', async () => {
    const res = await request(app).get('/api/auth/me');
    expect(res.status).toBe(401);
  });
});
