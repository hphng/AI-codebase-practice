const path = require('path');
const { execFileSync } = require('child_process');
const { planPickRoute } = require('../src/services/routePlanner');

const stop = (sku, row, col) => ({ sku, row, col });

describe('planPickRoute', () => {
  it('walks to the nearest stop by walking distance, then returns to S', () => {
    // S . # . .      C is 2 steps away, B is 4 from C, A is 3 from B, and it's 7 back to S.
    // . . # . #      (A looks close to C in a straight line, but the shelf is in the way.)
    // . . . . .
    const grid = ['S.#..', '..#.#', '.....'];
    expect(planPickRoute(grid, [stop('A', 0, 3), stop('B', 2, 4), stop('C', 1, 1)])).toEqual({
      order: ['C', 'B', 'A'],
      totalSteps: 16,
      unreachable: [],
    });
  });

  it('breaks distance ties by sku', () => {
    expect(planPickRoute(['.S.'], [stop('B', 0, 0), stop('A', 0, 2)])).toEqual({
      order: ['A', 'B'],
      totalSteps: 4,
      unreachable: [],
    });
  });

  it('picks several stops on the same cell without extra steps', () => {
    expect(planPickRoute(['.S.'], [stop('Y', 0, 2), stop('X', 0, 2)])).toEqual({
      order: ['X', 'Y'],
      totalSteps: 2,
      unreachable: [],
    });
  });

  it('reports stops that are walled off, on shelving or off the grid', () => {
    // P is walled off, Q is on a shelf, R is outside the grid.
    expect(planPickRoute(['S.#.', '..#.'], [stop('R', 5, 5), stop('P', 0, 3), stop('M', 1, 1), stop('Q', 1, 2)])).toEqual({
      order: ['M'],
      totalSteps: 4,
      unreachable: ['P', 'Q', 'R'],
    });
  });

  it('handles no stops and a stop on the dock itself', () => {
    expect(planPickRoute(['S..'], [])).toEqual({ order: [], totalSteps: 0, unreachable: [] });
    expect(planPickRoute(['S.'], [stop('K', 0, 0)])).toEqual({ order: ['K'], totalSteps: 0, unreachable: [] });
    expect(planPickRoute(['S#.'], [stop('Z', 0, 2)])).toEqual({ order: [], totalSteps: 0, unreachable: ['Z'] });
  });

  it('plans the seeded "Morning wave 1" route', () => {
    const grid = [
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
    const stops = [stop('SKU-1003', 1, 6), stop('SKU-1012', 5, 9), stop('SKU-1007', 2, 18), stop('SKU-1015', 5, 18)];
    expect(planPickRoute(grid, stops)).toEqual({
      order: ['SKU-1003', 'SKU-1012', 'SKU-1015', 'SKU-1007'],
      totalSteps: 48,
      unreachable: [],
    });
  });

  it('plans a 300 x 300 serpentine warehouse with 60 stops quickly', () => {
    // Timed in a plain Node process: Jest's sandbox makes some built-ins (e.g. Math.*) far slower
    // than they are in production, which would make the timing meaningless.
    const script = `
      const { grid, stops } = require(${JSON.stringify(path.join(__dirname, 'fixtures', 'serpentine.js'))});
      const { planPickRoute } = require(${JSON.stringify(path.join(__dirname, '..', 'src', 'services', 'routePlanner.js'))});
      const started = process.hrtime.bigint();
      const result = planPickRoute(grid, stops);
      const ms = Number(process.hrtime.bigint() - started) / 1e6;
      process.stdout.write(JSON.stringify({ ms, result }));
    `;
    const { ms, result } = JSON.parse(execFileSync(process.execPath, ['-e', script], { encoding: 'utf8', timeout: 60000 }));

    expect(result.order).toHaveLength(60);
    expect(result.order.slice(0, 5)).toEqual(['S057', 'S011', 'S037', 'S013', 'S043']);
    expect(result.totalSteps).toBe(89328);
    expect(ms).toBeLessThan(2000);
  });
});
