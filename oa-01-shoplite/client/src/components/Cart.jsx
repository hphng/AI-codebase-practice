import { useState } from 'react';
import { api, formatPrice } from '../api';
import { useCart } from '../state/CartContext.jsx';
import AlsoBought from './AlsoBought.jsx';

export default function Cart({ user, onNeedLogin }) {
  const { items, total, updateQuantity, removeItem, clear } = useCart();
  const [message, setMessage] = useState('');
  const [error, setError] = useState('');

  async function checkout() {
    if (!user) return onNeedLogin();
    setError('');
    try {
      const order = await api('/orders', {
        method: 'POST',
        body: { items: items.map((i) => ({ productId: i.productId, quantity: i.quantity })) },
      });
      clear();
      setMessage(`Order ${order._id} placed. Total ${formatPrice(order.total)}.`);
    } catch (err) {
      setError(err.message);
    }
  }

  if (items.length === 0) {
    return <section>{message ? <p className="ok">{message}</p> : <p>Your cart is empty.</p>}</section>;
  }

  return (
    <section>
      <table>
        <thead>
          <tr>
            <th>Product</th>
            <th>Price</th>
            <th>Qty</th>
            <th>Subtotal</th>
            <th />
          </tr>
        </thead>
        <tbody>
          {items.map((item) => (
            <tr key={item.productId}>
              <td>{item.name}</td>
              <td>{formatPrice(item.price)}</td>
              <td>
                <button onClick={() => updateQuantity(item.productId, item.quantity - 1)}>-</button>
                <span className="qty">{item.quantity}</span>
                <button onClick={() => updateQuantity(item.productId, item.quantity + 1)}>+</button>
              </td>
              <td>{formatPrice(item.price * item.quantity)}</td>
              <td>
                <button onClick={() => removeItem(item.productId)}>Remove</button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
      <p className="total">Total: {formatPrice(total)}</p>
      {error && <p className="error">{error}</p>}
      <button className="primary" onClick={checkout}>
        {user ? 'Place order' : 'Log in to check out'}
      </button>
      <AlsoBought productId={items[items.length - 1].productId} productName={items[items.length - 1].name} />
    </section>
  );
}
