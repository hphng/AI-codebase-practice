import { useEffect, useState } from 'react';
import { api, getToken, setToken } from './api';
import Login from './components/Login.jsx';
import Bins from './components/Bins.jsx';
import PickLists from './components/PickLists.jsx';

export default function App() {
  const [user, setUser] = useState(null);
  const [view, setView] = useState('picklists');

  useEffect(() => {
    if (!getToken()) return;
    api('/auth/me')
      .then(setUser)
      .catch(() => setToken(null));
  }, []);

  function handleLogin({ token, picker }) {
    setToken(token);
    setUser(picker);
  }

  function logout() {
    setToken(null);
    setUser(null);
  }

  if (!user) return <Login onLogin={handleLogin} />;

  return (
    <div className="app">
      <header>
        <h1>PickPath</h1>
        <nav>
          <button className={view === 'picklists' ? 'active' : ''} onClick={() => setView('picklists')}>
            Pick lists
          </button>
          <button className={view === 'bins' ? 'active' : ''} onClick={() => setView('bins')}>
            Bins
          </button>
          <span className="user">
            {user.name} ({user.role}) <button onClick={logout}>Log out</button>
          </span>
        </nav>
      </header>
      <main>
        {view === 'picklists' && <PickLists user={user} />}
        {view === 'bins' && <Bins user={user} />}
      </main>
    </div>
  );
}
