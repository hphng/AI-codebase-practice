export const initialCartState = { items: [] };

/**
 * Cart state: { items: [{ productId, name, price, quantity }] }
 *
 * Actions:
 *   ADD_ITEM         payload: { product, quantity? }
 *   UPDATE_QUANTITY  payload: { productId, quantity }   (quantity <= 0 removes the line)
 *   REMOVE_ITEM      payload: { productId }
 *   CLEAR
 */
export function cartReducer(state, action) {
  switch (action.type) {
    case 'ADD_ITEM': {
      const { product, quantity = 1 } = action.payload;
      const existing = state.items.find((item) => item.productId === product._id);

      if (existing) {
        existing.quantity += quantity;
        return { ...state };
      }

      return {
        ...state,
        items: [
          ...state.items,
          { productId: product._id, name: product.name, price: product.price, quantity },
        ],
      };
    }

    case 'UPDATE_QUANTITY': {
      const { productId, quantity } = action.payload;
      if (quantity <= 0) {
        return { ...state, items: state.items.filter((item) => item.productId !== productId) };
      }
      return {
        ...state,
        items: state.items.map((item) => (item.productId === productId ? { ...item, quantity } : item)),
      };
    }

    case 'REMOVE_ITEM':
      return {
        ...state,
        items: state.items.filter((item) => item.productId !== action.payload.productId),
      };

    case 'CLEAR':
      return initialCartState;

    default:
      return state;
  }
}

export function selectItemCount(state) {
  return state.items.reduce((count, item) => count + item.quantity, 0);
}

export function selectCartTotal(state) {
  const total = state.items.reduce((sum, item) => sum + item.price * item.quantity, 0);
  return Math.round(total * 100) / 100;
}
