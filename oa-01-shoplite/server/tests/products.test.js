const request = require('supertest');
const mongoose = require('mongoose');
const app = require('../src/app');
const Product = require('../src/models/Product');
const { createUser, createProduct } = require('./helpers');

const pad = (n) => String(n).padStart(2, '0');

describe('GET /api/products (catalog pagination)', () => {
  beforeEach(async () => {
    await Product.insertMany(
      Array.from({ length: 25 }, (_, i) => ({
        name: `Product ${pad(i + 1)}`,
        price: i + 1,
        stock: 5,
        category: i % 2 === 0 ? 'odd' : 'even',
      }))
    );
  });

  it('returns the first page sorted by name', async () => {
    const res = await request(app).get('/api/products?page=1&limit=10');

    expect(res.status).toBe(200);
    expect(res.body).toMatchObject({ page: 1, limit: 10, total: 25, totalPages: 3 });
    expect(res.body.items).toHaveLength(10);
    expect(res.body.items[0].name).toBe('Product 01');
    expect(res.body.items[9].name).toBe('Product 10');
  });

  it('returns a partial last page', async () => {
    const res = await request(app).get('/api/products?page=3&limit=10');

    expect(res.status).toBe(200);
    expect(res.body.items.map((p) => p.name)).toEqual([
      'Product 21',
      'Product 22',
      'Product 23',
      'Product 24',
      'Product 25',
    ]);
  });

  it('defaults to page 1 with 10 items', async () => {
    const res = await request(app).get('/api/products');

    expect(res.status).toBe(200);
    expect(res.body.page).toBe(1);
    expect(res.body.items[0].name).toBe('Product 01');
  });

  it('paging through every page visits every product exactly once', async () => {
    const seen = [];
    for (let page = 1; page <= 3; page++) {
      const res = await request(app).get(`/api/products?page=${page}&limit=10`);
      seen.push(...res.body.items.map((p) => p.name));
    }
    expect(seen).toHaveLength(25);
    expect(new Set(seen).size).toBe(25);
  });

  it('filters by category', async () => {
    const res = await request(app).get('/api/products?category=even&limit=100');

    expect(res.status).toBe(200);
    expect(res.body.total).toBe(12);
    expect(res.body.items.every((p) => p.category === 'even')).toBe(true);
  });
});

describe('GET /api/products/search', () => {
  beforeEach(async () => {
    await Product.insertMany([
      { name: 'Desk Lamp', price: 20, stock: 1 },
      { name: 'Floor LAMP', price: 60, stock: 1 },
      { name: 'Coffee Mug', price: 8, stock: 1 },
    ]);
  });

  it('finds products by case-insensitive partial name', async () => {
    const res = await request(app).get('/api/products/search?q=lamp');

    expect(res.status).toBe(200);
    expect(res.body.items.map((p) => p.name)).toEqual(['Desk Lamp', 'Floor LAMP']);
  });

  it('treats regex characters in the query literally', async () => {
    const res = await request(app).get('/api/products/search?q=' + encodeURIComponent('.*'));

    expect(res.status).toBe(200);
    expect(res.body.items).toEqual([]);
  });

  it('requires a query', async () => {
    const res = await request(app).get('/api/products/search');

    expect(res.status).toBe(400);
    expect(res.body.message).toMatch(/query/i);
  });
});

describe('GET /api/products/:id', () => {
  it('returns a product', async () => {
    const product = await createProduct({ name: 'Kettle' });
    const res = await request(app).get(`/api/products/${product._id}`);

    expect(res.status).toBe(200);
    expect(res.body.name).toBe('Kettle');
  });

  it('returns 404 for an unknown id', async () => {
    const res = await request(app).get(`/api/products/${new mongoose.Types.ObjectId()}`);
    expect(res.status).toBe(404);
  });

  it('returns 400 for a malformed id', async () => {
    const res = await request(app).get('/api/products/not-an-id');
    expect(res.status).toBe(400);
  });
});

describe('admin product management', () => {
  it('lets an admin create a product', async () => {
    const { auth } = await createUser({ role: 'admin' });
    const res = await request(app)
      .post('/api/products')
      .set(auth)
      .send({ name: 'Toaster', price: 25, stock: 4 });

    expect(res.status).toBe(201);
    expect(res.body.name).toBe('Toaster');
  });

  it('forbids customers from creating products', async () => {
    const { auth } = await createUser();
    const res = await request(app).post('/api/products').set(auth).send({ name: 'x', price: 1 });
    expect(res.status).toBe(403);
  });

  it('requires authentication to create products', async () => {
    const res = await request(app).post('/api/products').send({ name: 'x', price: 1 });
    expect(res.status).toBe(401);
  });

  it('serves fresh data after an admin updates a product', async () => {
    const { auth } = await createUser({ role: 'admin' });
    const product = await createProduct({ price: 10 });

    await request(app).get(`/api/products/${product._id}`); // warm the cache
    const patch = await request(app).patch(`/api/products/${product._id}`).set(auth).send({ price: 12.5 });
    expect(patch.status).toBe(200);

    const res = await request(app).get(`/api/products/${product._id}`);
    expect(res.body.price).toBe(12.5);
  });
});
