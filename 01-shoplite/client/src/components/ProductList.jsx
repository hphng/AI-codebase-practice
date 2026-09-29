import { useEffect, useState } from 'react';
import { api, formatPrice } from '../api';
import { useCart } from '../state/CartContext.jsx';

const PAGE_SIZE = 8;

export default function ProductList() {
  const { addItem } = useCart();
  const [page, setPage] = useState(1);
  const [data, setData] = useState({ items: [], totalPages: 1 });
  const [query, setQuery] = useState('');
  const [searchResults, setSearchResults] = useState(null);
  const [error, setError] = useState('');

  useEffect(() => {
    api(`/products?page=${page}&limit=${PAGE_SIZE}`)
      .then(setData)
      .catch((err) => setError(err.message));
  }, [page]);

  async function handleSearch(e) {
    e.preventDefault();
    setError('');
    if (!query.trim()) {
      setSearchResults(null);
      return;
    }
    try {
      const res = await api(`/products/search?q=${encodeURIComponent(query)}`);
      setSearchResults(res.items);
    } catch (err) {
      setError(err.message);
    }
  }

  const products = searchResults ?? data.items;

  return (
    <section>
      <form className="search" onSubmit={handleSearch}>
        <input value={query} onChange={(e) => setQuery(e.target.value)} placeholder="Search products" />
        <button type="submit">Search</button>
        {searchResults && (
          <button
            type="button"
            onClick={() => {
              setQuery('');
              setSearchResults(null);
            }}
          >
            Clear
          </button>
        )}
      </form>

      {error && <p className="error">{error}</p>}

      <ul className="grid">
        {products.map((p) => (
          <li key={p._id} className="card">
            <h3>{p.name}</h3>
            <p className="muted">{p.category}</p>
            <p className="price">{formatPrice(p.price)}</p>
            <p className="muted">{p.stock > 0 ? `${p.stock} in stock` : 'Out of stock'}</p>
            <button disabled={p.stock === 0} onClick={() => addItem(p)}>
              Add to cart
            </button>
          </li>
        ))}
      </ul>

      {!searchResults && (
        <div className="pager">
          <button disabled={page <= 1} onClick={() => setPage((n) => n - 1)}>
            Prev
          </button>
          <span>
            Page {page} of {data.totalPages || 1}
          </span>
          <button disabled={page >= data.totalPages} onClick={() => setPage((n) => n + 1)}>
            Next
          </button>
        </div>
      )}
    </section>
  );
}
