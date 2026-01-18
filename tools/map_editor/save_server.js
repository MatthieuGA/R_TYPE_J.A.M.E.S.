#!/usr/bin/env node
// Simple local save server to write exported levels into server/assets/worldgen/levels
// Usage: node save_server.js

const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT ? parseInt(process.env.PORT) : 4321;
const LEVELS_DIR = path.resolve(__dirname, '..', '..', 'server', 'assets', 'worldgen', 'levels');

function sendJSON(res, status, obj) {
  res.writeHead(status, { 'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*' });
  res.end(JSON.stringify(obj));
}

function ensureDir(dir) {
  if (!fs.existsSync(dir)) {
    fs.mkdirSync(dir, { recursive: true });
  }
}

ensureDir(LEVELS_DIR);

const server = http.createServer((req, res) => {
  // Allow simple CORS preflight
  if (req.method === 'OPTIONS') {
    res.writeHead(204, {
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'POST, OPTIONS',
      'Access-Control-Allow-Headers': 'Content-Type'
    });
    res.end();
    return;
  }

  if (req.method === 'GET' && req.url === '/health') {
    sendJSON(res, 200, { ok: true, dir: LEVELS_DIR });
    return;
  }

  if (req.method === 'POST' && req.url === '/save-level') {
    let body = '';
    req.on('data', chunk => { body += chunk; if (body.length > 5e6) req.socket.destroy(); });
    req.on('end', () => {
      try {
        const data = JSON.parse(body);
        if (!data || !data.filename || !data.content) {
          sendJSON(res, 400, { error: 'Missing filename or content' });
          return;
        }

        // Sanitize filename
        let filename = path.basename(data.filename);
        if (!filename.endsWith('.level.json')) filename = filename + '.level.json';

        const safePath = path.join(LEVELS_DIR, filename);

        // Prevent writing outside the levels dir
        if (!safePath.startsWith(LEVELS_DIR)) {
          sendJSON(res, 400, { error: 'Invalid filename' });
          return;
        }

        fs.writeFile(safePath, JSON.stringify(data.content, null, 2), { encoding: 'utf8' }, (err) => {
          if (err) {
            console.error('Failed to write file', err);
            sendJSON(res, 500, { error: 'Failed to write file', details: err.message });
            return;
          }
          console.log('Saved level to', safePath);
          sendJSON(res, 200, { ok: true, path: safePath });
        });
      } catch (err) {
        console.error('Invalid JSON', err);
        sendJSON(res, 400, { error: 'Invalid JSON', details: err.message });
      }
    });
    return;
  }

  sendJSON(res, 404, { error: 'Not found' });
});

server.listen(PORT, () => {
  console.log(`Save server listening on http://localhost:${PORT}`);
  console.log(`Will write levels into: ${LEVELS_DIR}`);
});
