const Picker = require('../src/models/Picker');
const Bin = require('../src/models/Bin');
const { signToken } = require('../src/utils/token');

let counter = 0;

async function createPicker({ role = 'picker', pin = '1111', badgeCode } = {}) {
  counter += 1;
  const picker = await Picker.create({
    name: `Picker ${counter}`,
    badgeCode: badgeCode || `BADGE-${String(counter).padStart(4, '0')}`,
    pin,
    role,
  });
  return { picker, auth: { Authorization: `Bearer ${signToken(picker)}` } };
}

function createBin(overrides = {}) {
  counter += 1;
  return Bin.create({
    code: `A01-${counter}`,
    sku: `SKU-${counter}`,
    productName: `Product ${counter}`,
    aisle: 1,
    row: 1,
    col: 0,
    quantity: 10,
    weightKg: 0.5,
    ...overrides,
  });
}

module.exports = { createPicker, createBin };
