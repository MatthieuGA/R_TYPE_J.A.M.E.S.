#!/usr/bin/env node
const { spawnSync } = require('child_process');
const fs = require('fs');

const fileArgs = process.argv.slice(2);
const filters = ['--repository=.', '--quiet', '--output=vs7', "--filter=-legal/copyright,-build/c++17,+build/c++20,-whitespace/indent_namespace,-runtime/references"];

const candidates = [
  ['py', ['-3', '-m', 'cpplint', ...filters]],
  ['python3', ['-m', 'cpplint', ...filters]],
  ['python', ['-m', 'cpplint', ...filters]],
  [process.platform === 'win32' ? '.\\.venv\\Scripts\\python.exe' : './.venv/bin/python', ['-m', 'cpplint', ...filters]]
];

function tryRun(cmd, args) {
  const fullArgs = args.concat(fileArgs);
  const res = spawnSync(cmd, fullArgs, { stdio: 'inherit' });
  return res && res.status === 0;
}

for (const [cmd, args] of candidates) {
  try {
    // if the candidate is a path, check existence first
    if ((cmd.includes('\\') || cmd.includes('/')) && !fs.existsSync(cmd)) continue;
    const ok = tryRun(cmd, args);
    if (ok) process.exit(0);
    // if cpplint not installed, detect that and print helpful message
  } catch (e) {
    // ignore and try next
  }
}

console.error('Could not run cpplint: no suitable Python + cpplint found.');
console.error('Install cpplint for one of these commands:');
console.error('  py -3 -m pip install --user cpplint');
console.error('  python3 -m pip install --user cpplint');
console.error('  python -m pip install --user cpplint');
process.exit(1);
