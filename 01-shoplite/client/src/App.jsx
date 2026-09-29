import { useEffect, useState } from 'react';
import { api, getToken, setToken } from './api';
import { useCart } from './state/CartContext.jsx';
import ProductList from './components/ProductList.jsx';
import Cart from './components/Cart.jsx';
import Orders from './components/Orders.jsx';
import Login from './components/Login.jsx';
import TopSellers from './components/TopSellers.jsx';

export default function App() {
  const [view, setView] = useState('products');
  const [user, setUser] = useState(null);
  const { count } = useCart();

  useEffect(() => {
    if (!getToken()) return;
    api('/auth/me')
      .then(setUser)
      .catch(() => setToken(null));
  }, []);

  function handleLogin({ token, user: loggedIn }) {
    setToken(token);
    setUser(loggedIn);
    setView('products');
  }

  function handleLogout() {
    setToken(null);
    setUser(null);
    setView('products');
  }

  const tabs = [
    ['products', 'Products'],
    ['cart', `Cart (${count})`],
    ...(user ? [['orders', 'My orders']] : []),
    ...(user?.role === 'admin' ? [['top', 'Top sellers']] : []),
  ];

  return (
    <div className="app">
      <header>
        <h1>ShopLite</h1>
        <nav>
          {tabs.map(([key, label]) => (
            <button key={key} className={view === key ? 'active' : ''} onClick={() => setView(key)}>
              {label}
            </button>
          ))}
          {user ? (
            <span className="user">
              {user.name} ({user.role}) <button onClick={handleLogout}>Log out</button>
            </span>
          ) : (
            <button className={view === 'login' ? 'active' : ''} onClick={() => setView('login')}>
              Log in
            </button>
          )}
        </nav>
      </header>

      <main>
        {view === 'products' && <ProductList />}
        {view === 'cart' && <Cart user={user} onNeedLogin={() => setView('login')} />}
        {view === 'orders' && user && <Orders />}
        {view === 'top' && user?.role === 'admin' && <TopSellers />}
        {view === 'login' && <Login onLogin={handleLogin} />}
      </main>
    </div>
  );
}
