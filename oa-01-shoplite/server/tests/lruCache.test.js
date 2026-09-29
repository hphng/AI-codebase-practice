const LRUCache = require('../src/utils/lruCache');

describe('LRUCache', () => {
  it('stores and retrieves values', () => {
    const cache = new LRUCache(2);
    cache.put('a', 1);
    expect(cache.get('a')).toBe(1);
    expect(cache.get('missing')).toBeUndefined();
  });

  it('evicts the least recently used entry when over capacity', () => {
    const cache = new LRUCache(2);
    cache.put('a', 1);
    cache.put('b', 2);
    cache.put('c', 3);

    expect(cache.has('a')).toBe(false);
    expect(cache.keys()).toEqual(['c', 'b']);
  });

  it('treats a read as a use', () => {
    const cache = new LRUCache(2);
    cache.put('a', 1);
    cache.put('b', 2);
    cache.get('a');
    cache.put('c', 3);

    expect(cache.has('b')).toBe(false);
    expect(cache.keys()).toEqual(['c', 'a']);
  });

  it('treats an overwrite as a use', () => {
    const cache = new LRUCache(2);
    cache.put('a', 1);
    cache.put('b', 2);
    cache.put('a', 10);
    cache.put('c', 3);

    expect(cache.get('a')).toBe(10);
    expect(cache.has('b')).toBe(false);
    expect(cache.keys()).toEqual(['a', 'c']);
  });

  it('never grows beyond capacity', () => {
    const cache = new LRUCache(3);
    for (let i = 0; i < 100; i++) cache.put(`k${i % 7}`, i);
    expect(cache.size).toBe(3);
  });

  it('deletes entries', () => {
    const cache = new LRUCache(2);
    cache.put('a', 1);
    expect(cache.delete('a')).toBe(true);
    expect(cache.delete('a')).toBe(false);
    expect(cache.size).toBe(0);
  });

  it('rejects an invalid capacity', () => {
    expect(() => new LRUCache(0)).toThrow(RangeError);
    expect(() => new LRUCache(1.5)).toThrow(RangeError);
  });
});
