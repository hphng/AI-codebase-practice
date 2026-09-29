import { describe, it, expect } from 'vitest';
import { cartReducer, initialCartState, selectCartTotal, selectItemCount } from './cartReducer';

const lamp = { _id: 'p1', name: 'Desk Lamp', price: 19.99 };
const mug = { _id: 'p2', name: 'Mug', price: 7.5 };
const add = (product, quantity) => ({ type: 'ADD_ITEM', payload: { product, quantity } });

describe('cartReducer', () => {
  it('adds a new product as a line', () => {
    const state = cartReducer(initialCartState, add(lamp));
    expect(state.items).toEqual([{ productId: 'p1', name: 'Desk Lamp', price: 19.99, quantity: 1 }]);
  });

  it('increments quantity when the product is already in the cart', () => {
    let state = cartReducer(initialCartState, add(lamp));
    state = cartReducer(state, add(lamp, 2));
    expect(state.items).toHaveLength(1);
    expect(state.items[0].quantity).toBe(3);
  });

  it('does not modify the previous state', () => {
    const before = cartReducer(initialCartState, add(lamp));
    const snapshot = JSON.parse(JSON.stringify(before));

    const after = cartReducer(before, add(lamp));

    expect(before).toEqual(snapshot);
    expect(after.items).not.toBe(before.items);
    expect(after.items[0]).not.toBe(before.items[0]);
  });

  it('is pure: same state + same action gives the same result every time', () => {
    const state = cartReducer(initialCartState, add(lamp));
    const first = cartReducer(state, add(lamp));
    const second = cartReducer(state, add(lamp));

    expect(first.items[0].quantity).toBe(2);
    expect(second.items[0].quantity).toBe(2);
  });

  it('updates and removes lines', () => {
    let state = cartReducer(initialCartState, add(lamp));
    state = cartReducer(state, add(mug));
    state = cartReducer(state, { type: 'UPDATE_QUANTITY', payload: { productId: 'p2', quantity: 4 } });
    expect(state.items[1].quantity).toBe(4);

    state = cartReducer(state, { type: 'UPDATE_QUANTITY', payload: { productId: 'p2', quantity: 0 } });
    expect(state.items.map((i) => i.productId)).toEqual(['p1']);

    state = cartReducer(state, { type: 'REMOVE_ITEM', payload: { productId: 'p1' } });
    expect(state.items).toEqual([]);
  });

  it('computes count and total', () => {
    let state = cartReducer(initialCartState, add(lamp, 2));
    state = cartReducer(state, add(mug, 1));
    expect(selectItemCount(state)).toBe(3);
    expect(selectCartTotal(state)).toBe(47.48);
  });
});
