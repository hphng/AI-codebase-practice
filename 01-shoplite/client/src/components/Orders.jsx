import { useEffect, useState } from 'react';
import { api, formatPrice } from '../api';

export default function Orders() {
  const [orders, setOrders] = useState([]);
  const [selected, setSelected] = useState(null);
  const [error, setError] = useState('');

  function load() {
    api('/orders/mine')
      .then(setOrders)
      .catch((err) => setError(err.message));
  }

  useEffect(load, []);

  async function openOrder(id) {
    setError('');
    try {
      setSelected(await api(`/orders/${id}`));
    } catch (err) {
      setSelected(null);
      setError(err.message);
    }
  }

  async function cancel(id) {
    setError('');
    try {
      setSelected(await api(`/orders/${id}/cancel`, { method: 'POST' }));
    } catch (err) {
      setError(err.message);
    }
    load();
  }

  return (
    <section className="orders">
      {error && <p className="error">{error}</p>}
      {orders.length === 0 && <p>No orders yet.</p>}
      <ul>
        {orders.map((o) => (
          <li key={o._id}>
            <button className="link" onClick={() => openOrder(o._id)}>
              {new Date(o.createdAt).toLocaleString()} - {formatPrice(o.total)} - {o.status}
            </button>
          </li>
        ))}
      </ul>

      {selected && (
        <div className="card detail">
          <h3>Order {selected._id}</h3>
          <p>Status: {selected.status}</p>
          <ul>
            {selected.items.map((i) => (
              <li key={i.product}>
                {i.quantity} x {i.name} @ {formatPrice(i.price)}
              </li>
            ))}
          </ul>
          <p className="total">Total: {formatPrice(selected.total)}</p>
          {selected.status !== 'cancelled' && <button onClick={() => cancel(selected._id)}>Cancel order</button>}
        </div>
      )}
    </section>
  );
}
