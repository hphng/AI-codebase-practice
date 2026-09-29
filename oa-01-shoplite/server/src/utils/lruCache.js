class Node {
  constructor(key, value) {
    this.key = key;
    this.value = value;
    this.prev = null;
    this.next = null;
  }
}

/**
 * Least-recently-used cache backed by a hash map + doubly linked list.
 * All operations are O(1).
 *
 * List layout:  head <-> [most recent] <-> ... <-> [least recent] <-> tail
 * head and tail are sentinels and never hold data.
 */
class LRUCache {
  constructor(capacity) {
    if (!Number.isInteger(capacity) || capacity <= 0) {
      throw new RangeError('LRUCache capacity must be a positive integer');
    }
    this.capacity = capacity;
    this.map = new Map();
    this.head = new Node(null, null);
    this.tail = new Node(null, null);
    this.head.next = this.tail;
    this.tail.prev = this.head;
  }

  _unlink(node) {
    node.prev.next = node.next;
    node.next.prev = node.prev;
    node.prev = null;
    node.next = null;
  }

  _pushFront(node) {
    node.prev = this.head;
    node.next = this.head.next;
    this.head.next.prev = node;
    this.head.next = node;
  }

  get(key) {
    const node = this.map.get(key);
    if (!node) return undefined;
    this._unlink(node);
    this._pushFront(node);
    return node.value;
  }

  put(key, value) {
    const existing = this.map.get(key);
    if (existing) {
      existing.value = value;
      this._unlink(existing);
      this._pushFront(existing);
      return;
    }

    const node = new Node(key, value);
    this.map.set(key, node);
    this._pushFront(node);

    if (this.map.size > this.capacity) {
      const lru = this.tail.prev;
      this._unlink(lru);
      this.map.delete(lru.key);
    }
  }

  delete(key) {
    const node = this.map.get(key);
    if (!node) return false;
    this._unlink(node);
    this.map.delete(key);
    return true;
  }

  has(key) {
    return this.map.has(key);
  }

  clear() {
    this.map.clear();
    this.head.next = this.tail;
    this.tail.prev = this.head;
  }

  get size() {
    return this.map.size;
  }

  /** Keys ordered from most to least recently used. */
  keys() {
    const out = [];
    for (let node = this.head.next; node !== this.tail; node = node.next) out.push(node.key);
    return out;
  }
}

module.exports = LRUCache;
