const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const PORT = 3000;
const ROOT = path.join(__dirname, '..');
const EXE  = path.join(ROOT, 'code.exe');
const DATA = path.join(__dirname, 'data');

if (!fs.existsSync(DATA)) fs.mkdirSync(DATA, { recursive: true });

let child = null;
let buf = '';

function startChild() {
  if (child) { try { child.kill(); } catch (e) {} }
  buf = '';
  child = spawn(EXE, [], { cwd: DATA });
  child.stdout.on('data', d => { buf += d.toString('utf8'); });
  child.stderr.on('data', d => { buf += d.toString('utf8'); });
  child.on('exit', () => { child = null; });
}

startChild();

function sendCmd(cmd) {
  return new Promise((resolve) => {
    if (!child) startChild();
    buf = '';
    child.stdin.write(cmd + '\n');
    setTimeout(() => resolve(buf), 60);   // collect output
  });
}

const MIME = { '.html': 'text/html; charset=utf-8', '.js': 'text/javascript', '.css': 'text/css' };

const server = http.createServer((req, res) => {
  if (req.method === 'POST' && req.url === '/api/cmd') {
    let body = '';
    req.on('data', c => body += c);
    req.on('end', async () => {
      try {
        const { cmd } = JSON.parse(body);
        const out = await sendCmd(String(cmd));
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ ok: true, out }));
      } catch (e) {
        res.writeHead(500); res.end(String(e));
      }
    });
    return;
  }
  if (req.method === 'POST' && req.url === '/api/reset') {
    startChild();
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ ok: true }));
    return;
  }
  // static
  let p = req.url === '/' ? '/index.html' : req.url.split('?')[0];
  let f = path.join(__dirname, 'public', p);
  fs.readFile(f, (err, data) => {
    if (err) { res.writeHead(404); res.end('not found'); return; }
    res.writeHead(200, { 'Content-Type': MIME[path.extname(f)] || 'application/octet-stream' });
    res.end(data);
  });
});

server.listen(PORT, () => {
  console.log(`Bookstore UI running at http://localhost:${PORT}`);
});
