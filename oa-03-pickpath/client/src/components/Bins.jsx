import { useEffect, useState } from 'react';
import { api } from '../api';
import { available, groupByAisle } from '../utils/bins';

export default function Bins({ user }) {
  const [bins, setBins] = useState([]);
  const [error, setError] = useState('');
  const [drafts, setDrafts] = useState({});

  useEffect(() => {
    api('/bins')
      .then(setBins)
      .catch((err) => setError(err.message));
  }, []);

  async function saveCount(bin) {
    setError('');
    try {
      const updated = await api(`/bins/${bin._id}`, {
        method: 'PATCH',
        body: { quantity: Number(drafts[bin._id]) },
      });
      setBins((all) => all.map((b) => (b._id === updated._id ? updated : b)));
      setDrafts((d) => ({ ...d, [bin._id]: undefined }));
    } catch (err) {
      setError(err.message);
    }
  }

  return (
    <section>
      {error && <p className="error">{error}</p>}
      {groupByAisle(bins).map((group) => (
        <div key={group.aisle} className="card aisle">
          <h3>Aisle {group.aisle}</h3>
          <table>
            <thead>
              <tr>
                <th>Bin</th>
                <th>SKU</th>
                <th>Product</th>
                <th>Unit kg</th>
                <th>On hand</th>
                {user.role === 'lead' && <th>Cycle count</th>}
              </tr>
            </thead>
            <tbody>
              {group.bins.map((bin) => (
                <tr key={bin._id}>
                  <td>{bin.code}</td>
                  <td>{bin.sku}</td>
                  <td>{bin.productName}</td>
                  <td>{bin.weightKg}</td>
                  <td>{available(bin)}</td>
                  {user.role === 'lead' && (
                    <td>
                      <input
                        className="count"
                        value={drafts[bin._id] ?? ''}
                        placeholder={String(bin.quantity)}
                        onChange={(e) => setDrafts((d) => ({ ...d, [bin._id]: e.target.value }))}
                      />
                      <button disabled={drafts[bin._id] === undefined || drafts[bin._id] === ''} onClick={() => saveCount(bin)}>
                        Save
                      </button>
                    </td>
                  )}
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      ))}
    </section>
  );
}
