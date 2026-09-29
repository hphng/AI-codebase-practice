import { useEffect, useState } from 'react';
import { api } from '../api';
import WarehouseMap from './WarehouseMap.jsx';

// Parses lines like "SKU-1003 x2" into { sku, quantity }.
function parseItems(text) {
  return text
    .split('\n')
    .map((line) => line.trim())
    .filter(Boolean)
    .map((line) => {
      const [sku, qty] = line.split(/\s+x/i);
      return { sku: sku.trim(), quantity: Number(qty || 1) };
    });
}

export default function PickLists({ user }) {
  const [lists, setLists] = useState([]);
  const [route, setRoute] = useState(null);
  const [error, setError] = useState('');
  const [form, setForm] = useState({ name: '', toteMaxKg: '5', items: 'SKU-1001 x2\nSKU-1008 x1' });

  function load() {
    api('/picklists')
      .then(setLists)
      .catch((err) => setError(err.message));
  }

  useEffect(load, []);

  async function plan(list) {
    setError('');
    setRoute(null);
    try {
      setRoute({ name: list.name, ...(await api(`/picklists/${list._id}/route`, { method: 'POST' })) });
    } catch (err) {
      setError(`Route planning failed: ${err.message}`);
    }
  }

  async function create(e) {
    e.preventDefault();
    setError('');
    try {
      await api('/picklists', {
        method: 'POST',
        body: { name: form.name, toteMaxKg: Number(form.toteMaxKg), items: parseItems(form.items) },
      });
      setForm({ ...form, name: '' });
      load();
    } catch (err) {
      setError(err.message);
    }
  }

  return (
    <section className="picklists">
      {error && <p className="error">{error}</p>}
      <table>
        <thead>
          <tr>
            <th>Name</th>
            <th>Items</th>
            <th>Weight</th>
            <th />
          </tr>
        </thead>
        <tbody>
          {lists.map((list) => (
            <tr key={list._id}>
              <td>{list.name}</td>
              <td>{list.items.map((i) => `${i.sku} x${i.quantity}`).join(', ')}</td>
              <td>
                {list.totalKg} / {list.toteMaxKg} kg
              </td>
              <td>
                <button onClick={() => plan(list)}>Plan route</button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>

      {route && (
        <div className="card">
          <h3>Route for {route.name}</h3>
          <p>
            Visit order: <strong>{route.order.join(' → ') || '(nothing reachable)'}</strong>, total{' '}
            <strong>{route.totalSteps}</strong> steps (back to dock included).
          </p>
          {route.unreachable.length > 0 && <p className="error">Unreachable: {route.unreachable.join(', ')}</p>}
          <WarehouseMap grid={route.grid} stops={route.stops} order={route.order} />
        </div>
      )}

      {user.role === 'lead' && (
        <form className="card create" onSubmit={create}>
          <h3>New pick list</h3>
          <input placeholder="Name" value={form.name} onChange={(e) => setForm({ ...form, name: e.target.value })} />
          <label>
            Tote limit (kg){' '}
            <input value={form.toteMaxKg} onChange={(e) => setForm({ ...form, toteMaxKg: e.target.value })} />
          </label>
          <textarea rows={4} value={form.items} onChange={(e) => setForm({ ...form, items: e.target.value })} />
          <button className="primary" type="submit">
            Create
          </button>
        </form>
      )}
    </section>
  );
}
