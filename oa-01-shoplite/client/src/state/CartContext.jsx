import { createContext, useContext, useMemo, useReducer } from 'react';
import { cartReducer, initialCartState, selectCartTotal, selectItemCount } from './cartReducer';

const CartContext = createContext(null);

export function CartProvider({ children }) {
  const [state, dispatch] = useReducer(cartReducer, initialCartState);

  const value = useMemo(
    () => ({
      items: state.items,
      count: selectItemCount(state),
      total: selectCartTotal(state),
      addItem: (product, quantity = 1) => dispatch({ type: 'ADD_ITEM', payload: { product, quantity } }),
      updateQuantity: (productId, quantity) =>
        dispatch({ type: 'UPDATE_QUANTITY', payload: { productId, quantity } }),
      removeItem: (productId) => dispatch({ type: 'REMOVE_ITEM', payload: { productId } }),
      clear: () => dispatch({ type: 'CLEAR' }),
    }),
    [state]
  );

  return <CartContext.Provider value={value}>{children}</CartContext.Provider>;
}

export function useCart() {
  const ctx = useContext(CartContext);
  if (!ctx) throw new Error('useCart must be used inside <CartProvider>');
  return ctx;
}
