import { useEffect, useState } from 'react';
import { api, formatPrice } from '../api';

export default function TopSellers() {
  const [rows, setRows] = useState([]);
  const [error, setError] = useState('');

  useEffect(() => {
    api('/analytics/top-products?k=5')
      .then(setRows)
      .catch((err) => setError(err.message));
  }, []);

  return (
    <section>
      {error && <p className="error">{error}</p>}
      <table>
        <thead>
          <tr>
            <th>#</th>
            <th>Product</th>
            <th>Units sold</th>
            <th>Revenue</th>
          </tr>
        </thead>
        <tbody>
          {rows.map((r, i) => (
            <tr key={r.productId}>
              <td>{i + 1}</td>
              <td>{r.name}</td>
              <td>{r.unitsSold}</td>
              <td>{formatPrice(r.revenue)}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </section>
  );
}
