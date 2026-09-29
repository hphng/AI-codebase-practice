const AppError = require('../utils/AppError');

/**
 * PART A: implement this function.
 *
 * Plans the walking route for one pick list.
 *
 * @param {string[]} grid  floor plan, one string per row, all rows the same length:
 *                         'S' = dock (exactly one), '.' = walkable floor, '#' = shelving (blocked)
 * @param {Array<{ sku: string, row: number, col: number }>} stops
 *                         pick faces to visit. (row, col) is the cell the picker must stand on.
 * @returns {{ order: string[], totalSteps: number, unreachable: string[] }}
 *
 * Rules
 *  1. The picker starts on 'S' and moves up/down/left/right, one cell per step. They can't enter
 *     '#' cells or leave the grid. 'S' and '.' cells are walkable.
 *  2. Greedy route: from the current cell, always walk next to the remaining stop that is the
 *     FEWEST STEPS away (real walking distance around shelves, not a straight line). On a tie,
 *     pick the smaller sku (string comparison). The picker is then standing on that stop's cell.
 *  3. Several stops may share a cell. Once the picker is there, the others are 0 steps away.
 *  4. A stop is unreachable if its cell is outside the grid, is a '#', or can't be reached from 'S'.
 *     Unreachable stops are skipped and reported in `unreachable`, sorted by sku.
 *  5. After the last reachable stop, the picker walks back to 'S'. `totalSteps` counts every
 *     step, including the walk back. With no reachable stops: order = [], totalSteps = 0.
 *  6. `order` lists the skus of reachable stops in the order they are visited.
 *
 * Performance
 *  Grids up to 300 x 300 with up to 60 stops must be planned in well under a second.
 *  There is a performance test.
 *
 * Examples: see tests/routePlanner.test.js
 */
function planPickRoute(grid, stops) {
  throw new AppError(501, 'planPickRoute is not implemented yet');
}

module.exports = { planPickRoute };
