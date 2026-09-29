// 300 x 300 warehouse where every other row is a wall with a single gap at alternating ends,
// so walking distances are long "snake" paths. 60 pseudo-random stops on the open rows.
const N = 300;

const grid = [];
for (let r = 0; r < N; r++) {
  if (r % 2 === 0) grid.push('.'.repeat(N));
  else grid.push(r % 4 === 1 ? '.' + '#'.repeat(N - 1) : '#'.repeat(N - 1) + '.');
}
grid[0] = 'S' + grid[0].slice(1);

let seed = 12345;
const rnd = () => (seed = (seed * 1103515245 + 12345) % 2147483648);
const stops = [];
for (let i = 0; i < 60; i++) {
  stops.push({ sku: `S${String(i).padStart(3, '0')}`, row: 2 * (rnd() % 150), col: rnd() % N });
}

module.exports = { grid, stops };
