const Picker = require('../models/Picker');
const Bin = require('../models/Bin');
const PickList = require('../models/PickList');
const Warehouse = require('../models/Warehouse');

// 9 x 24 floor. Walkable cross-aisles on rows 0, 4, 8; shelving pairs with a walkable
// aisle on every third column. Aisles 1-8 are in the top block, 9-16 in the bottom block.
const GRID = [
  'S.......................',
  '.##.##.##.##.##.##.##.##',
  '.##.##.##.##.##.##.##.##',
  '.##.##.##.##.##.##.##.##',
  '........................',
  '.##.##.##.##.##.##.##.##',
  '.##.##.##.##.##.##.##.##',
  '.##.##.##.##.##.##.##.##',
  '........................',
];

const PRODUCTS = [
  ['USB-C Cable', 0.05], ['Phone Case', 0.08], ['Paperback Novel', 0.3], ['Coffee Beans 1kg', 1.0],
  ['Desk Lamp', 1.25], ['Yoga Mat', 1.1], ['Water Bottle', 0.35], ['Wireless Mouse', 0.1],
  ['Cast Iron Pan', 2.4], ['Board Game', 1.3], ['LED Bulb 4-pack', 0.2], ['Kettle', 1.15],
  ['Toaster', 2.2], ['Notebook A5', 0.25], ['Bluetooth Speaker', 0.6], ['Headphones', 0.3],
];

function binFor(index) {
  const aisle = index + 1;
  const top = aisle <= 8;
  const col = 3 * ((aisle - 1) % 8);
  const row = (top ? 1 : 5) + (aisle % 3);
  const [productName, weightKg] = PRODUCTS[index];
  return {
    code: `A${String(aisle).padStart(2, '0')}-${(aisle % 3) + 1}`,
    sku: `SKU-${1001 + index}`,
    productName,
    aisle,
    row,
    col,
    quantity: 20 + ((index * 7) % 30),
    weightKg,
  };
}

async function seed() {
  await Promise.all([Picker.deleteMany({}), Bin.deleteMany({}), PickList.deleteMany({}), Warehouse.deleteMany({})]);

  const [lead] = await Picker.create([
    { name: 'Dana (lead)', badgeCode: 'LEAD-0001', pin: '1234', role: 'lead' },
    { name: 'Marco', badgeCode: 'PICK-0042', pin: '4242', role: 'picker' },
  ]);

  await Warehouse.create({ name: 'FC-DEMO', grid: GRID });
  await Bin.insertMany(PRODUCTS.map((_, i) => binFor(i)));

  await PickList.create([
    {
      name: 'Morning wave 1',
      toteMaxKg: 15,
      totalKg: 3.3,
      items: [
        { sku: 'SKU-1003', quantity: 2 },
        { sku: 'SKU-1012', quantity: 1 },
        { sku: 'SKU-1007', quantity: 1 },
        { sku: 'SKU-1015', quantity: 2 },
      ],
      createdBy: lead._id,
    },
    {
      name: 'Kitchen restock',
      toteMaxKg: 20,
      totalKg: 10.25,
      items: [
        { sku: 'SKU-1009', quantity: 2 },
        { sku: 'SKU-1013', quantity: 1 },
        { sku: 'SKU-1004', quantity: 2 },
        { sku: 'SKU-1005', quantity: 1 },
      ],
      createdBy: lead._id,
    },
  ]);

  console.log('[seed] badges: LEAD-0001 / PIN 1234 (lead), PICK-0042 / PIN 4242 (picker)');
}

module.exports = { seed, GRID };
