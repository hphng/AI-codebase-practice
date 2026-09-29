/**
 * Groups bins for the floor view: one group per aisle, aisles in walking order
 * (1, 2, 3, ... 10, 11 ...), and bins inside an aisle ordered by bin code.
 *
 * @param {Array<{ aisle: number, code: string }>} bins
 * @returns {Array<{ aisle: number, bins: object[] }>}
 */
export function groupByAisle(bins) {
  const groups = new Map();
  for (const bin of bins) {
    if (!groups.has(bin.aisle)) groups.set(bin.aisle, []);
    groups.get(bin.aisle).push(bin);
  }

  const aisles = [...groups.keys()].sort();
  return aisles.map((aisle) => ({
    aisle,
    bins: groups.get(aisle).sort((a, b) => a.code.localeCompare(b.code)),
  }));
}

/** Units that can still be picked, never negative. */
export function available(bin) {
  return Math.max(0, bin.quantity);
}
