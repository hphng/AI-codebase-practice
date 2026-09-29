import { useEffect, useState } from 'react';
import { api, formatPrice } from '../api';
import { useCart } from '../state/CartContext.jsx';

// "Frequently bought together" suggestions for one product. Renders nothing if the
// endpoint fails (e.g. 501 while the recommendation algorithm isn't implemented).
export default function AlsoBought({ productId, productName }) {
  const { addItem } = useCart();
  const [items, setItems] = useState([]);

  useEffect(() => {
    let cancelled = false;
    api(`/products/${productId}/also-bought?k=3`)
      .then((rows) => !cancelled && setItems(rows))
      .catch(() => !cancelled && setItems([]));
    return () => {
      cancelled = true;
    };
  }, [productId]);

  if (items.length === 0) return null;

  return (
    <div className="card also-bought">
      <h3>Frequently bought with {productName}</h3>
      <ul>
        {items.map((r) => (
          <li key={r.productId}>
            {r.name} - {formatPrice(r.price)} <span className="muted">({r.score} orders)</span>{' '}
            <button
              disabled={r.stock === 0}
              onClick={() => addItem({ _id: r.productId, name: r.name, price: r.price })}
            >
              Add
            </button>
          </li>
        ))}
      </ul>
    </div>
  );
}
