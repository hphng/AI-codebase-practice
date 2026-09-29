import { describe, it, expect } from 'vitest';
import { groupByAisle } from './bins';

const bin = (code, aisle) => ({ code, aisle, quantity: 1 });

describe('groupByAisle', () => {
  it('groups bins by aisle', () => {
    const groups = groupByAisle([bin('A01-2', 1), bin('A02-1', 2), bin('A01-1', 1)]);
    expect(groups.map((g) => g.aisle)).toEqual([1, 2]);
    expect(groups[0].bins.map((b) => b.code)).toEqual(['A01-1', 'A01-2']);
  });

  it('orders aisles in walking order, including double-digit aisles', () => {
    const groups = groupByAisle([bin('A10-1', 10), bin('A02-1', 2), bin('A16-1', 16), bin('A01-1', 1), bin('A09-1', 9)]);
    expect(groups.map((g) => g.aisle)).toEqual([1, 2, 9, 10, 16]);
  });

  it('returns [] for no bins', () => {
    expect(groupByAisle([])).toEqual([]);
  });
});
