import { useState } from 'react';
import { api } from '../api';

export default function Login({ onLogin }) {
  const [badgeCode, setBadgeCode] = useState('');
  const [pin, setPin] = useState('');
  const [error, setError] = useState('');

  async function submit(e) {
    e.preventDefault();
    setError('');
    try {
      onLogin(await api('/auth/login', { method: 'POST', body: { badgeCode, pin } }));
    } catch (err) {
      setError(err.message);
    }
  }

  return (
    <div className="app">
      <h1>PickPath</h1>
      <form className="login" onSubmit={submit}>
        <input placeholder="Badge code" value={badgeCode} onChange={(e) => setBadgeCode(e.target.value)} />
        <input placeholder="PIN" type="password" value={pin} onChange={(e) => setPin(e.target.value)} />
        <button className="primary" type="submit">
          Log in
        </button>
        {error && <p className="error">{error}</p>}
      </form>
      <p className="muted">Demo: LEAD-0001 / 1234 (lead) - PICK-0042 / 4242 (picker)</p>
    </div>
  );
}
