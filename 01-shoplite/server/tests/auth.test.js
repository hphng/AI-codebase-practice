const request = require('supertest');
const jwt = require('jsonwebtoken');
const app = require('../src/app');
const config = require('../src/config');
const { createUser } = require('./helpers');

describe('POST /api/auth/register', () => {
  it('creates a customer account and returns a token', async () => {
    const res = await request(app)
      .post('/api/auth/register')
      .send({ name: 'Ann', email: 'Ann@Example.com', password: 'secret12' });

    expect(res.status).toBe(201);
    expect(res.body.token).toEqual(expect.any(String));
    expect(res.body.user).toMatchObject({ name: 'Ann', email: 'ann@example.com', role: 'customer' });
    expect(res.body.user.password).toBeUndefined();
  });

  it('never lets a caller register as admin', async () => {
    const res = await request(app)
      .post('/api/auth/register')
      .send({ name: 'Eve', email: 'eve@example.com', password: 'secret12', role: 'admin' });

    expect(res.status).toBe(201);
    expect(res.body.user.role).toBe('customer');
  });

  it('rejects a duplicate email', async () => {
    const body = { name: 'Ann', email: 'ann@example.com', password: 'secret12' };
    await request(app).post('/api/auth/register').send(body);
    const res = await request(app).post('/api/auth/register').send(body);

    expect(res.status).toBe(409);
  });

  it('rejects missing fields', async () => {
    const res = await request(app).post('/api/auth/register').send({ email: 'a@b.c' });
    expect(res.status).toBe(400);
  });
});

describe('POST /api/auth/login', () => {
  beforeEach(async () => {
    await request(app)
      .post('/api/auth/register')
      .send({ name: 'Ann', email: 'ann@example.com', password: 'secret12' });
  });

  it('returns a token for valid credentials', async () => {
    const res = await request(app)
      .post('/api/auth/login')
      .send({ email: 'ann@example.com', password: 'secret12' });

    expect(res.status).toBe(200);
    expect(res.body.token).toEqual(expect.any(String));
  });

  it('rejects a wrong password', async () => {
    const res = await request(app)
      .post('/api/auth/login')
      .send({ email: 'ann@example.com', password: 'wrong-password' });

    expect(res.status).toBe(401);
  });
});

describe('GET /api/auth/me (token validation)', () => {
  it('returns the current user for a valid token', async () => {
    const { user, auth } = await createUser();
    const res = await request(app).get('/api/auth/me').set(auth);

    expect(res.status).toBe(200);
    expect(res.body.email).toBe(user.email);
  });

  it('rejects a request without a token', async () => {
    const res = await request(app).get('/api/auth/me');
    expect(res.status).toBe(401);
  });

  it('rejects a garbage token', async () => {
    const res = await request(app).get('/api/auth/me').set({ Authorization: 'Bearer not.a.jwt' });
    expect(res.status).toBe(401);
  });

  it('rejects a token signed with a different secret', async () => {
    const { user } = await createUser();
    const forged = jwt.sign({ id: String(user._id), role: 'customer' }, 'attacker-secret');

    const res = await request(app).get('/api/auth/me').set({ Authorization: `Bearer ${forged}` });
    expect(res.status).toBe(401);
  });

  it('rejects an expired token', async () => {
    const { user } = await createUser();
    const expired = jwt.sign(
      { id: String(user._id), role: 'customer', exp: Math.floor(Date.now() / 1000) - 60 },
      config.jwtSecret
    );

    const res = await request(app).get('/api/auth/me').set({ Authorization: `Bearer ${expired}` });
    expect(res.status).toBe(401);
  });

  it('does not let a forged token escalate to admin', async () => {
    const { user } = await createUser();
    const forged = jwt.sign({ id: String(user._id), role: 'admin' }, 'attacker-secret');

    const res = await request(app)
      .post('/api/products')
      .set({ Authorization: `Bearer ${forged}` })
      .send({ name: 'Free stuff', price: 0, stock: 999 });

    expect(res.status).toBe(401);
  });
});
