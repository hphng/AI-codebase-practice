import { useState } from 'react';
import { api } from '../api';

export default function Login({ onLogin }) {
  const [mode, setMode] = useState('login');
  const [form, setForm] = useState({ name: '', email: '', password: '' });
  const [error, setError] = useState('');

  const update = (field) => (e) => setForm({ ...form, [field]: e.target.value });

  async function submit(e) {
    e.preventDefault();
    setError('');
    try {
      const path = mode === 'login' ? '/auth/login' : '/auth/register';
      onLogin(await api(path, { method: 'POST', body: form }));
    } catch (err) {
      setError(err.message);
    }
  }

  return (
    <section className="login">
      <form onSubmit={submit}>
        {mode === 'register' && <input placeholder="Name" value={form.name} onChange={update('name')} />}
        <input placeholder="Email" value={form.email} onChange={update('email')} />
        <input placeholder="Password" type="password" value={form.password} onChange={update('password')} />
        <button className="primary" type="submit">
          {mode === 'login' ? 'Log in' : 'Create account'}
        </button>
        {error && <p className="error">{error}</p>}
      </form>
      <button className="link" onClick={() => setMode(mode === 'login' ? 'register' : 'login')}>
        {mode === 'login' ? 'Need an account? Register' : 'Have an account? Log in'}
      </button>
      <p className="muted">Demo: alice@shoplite.dev / alice123 - admin@shoplite.dev / admin123</p>
    </section>
  );
}
