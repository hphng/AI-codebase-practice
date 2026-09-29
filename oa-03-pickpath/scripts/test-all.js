// Runs server (Jest) and client (Vitest) suites and prints a combined verdict.
const { spawnSync } = require('child_process');
const path = require('path');

const run = (dir) =>
  spawnSync('npm', ['test'], { cwd: path.join(__dirname, '..', dir), stdio: 'inherit', shell: true }).status;

const server = run('server');
const client = run('client');

const label = (code) => (code === 0 ? 'PASS' : 'FAIL');
console.log(`\n==== server: ${label(server)}   client: ${label(client)} ====`);
process.exit(server === 0 && client === 0 ? 0 : 1);
