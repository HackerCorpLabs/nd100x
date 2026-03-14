#!/usr/bin/env node
// Static file server with COOP/COEP headers for SharedArrayBuffer support.
// Replaces the Python SimpleHTTPRequestHandler with proper MIME types,
// gzip/deflate compression, and range request support.

import { createServer } from 'node:http';
import { createReadStream, statSync } from 'node:fs';
import { join, extname } from 'node:path';
import { createGzip, createDeflate } from 'node:zlib';

var port = parseInt(process.argv[2]) || 8000;
var root = process.argv[3] || process.cwd();

var MIME = {
  '.html': 'text/html; charset=utf-8',
  '.css':  'text/css; charset=utf-8',
  '.js':   'application/javascript; charset=utf-8',
  '.mjs':  'application/javascript; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.wasm': 'application/wasm',
  '.png':  'image/png',
  '.jpg':  'image/jpeg',
  '.jpeg': 'image/jpeg',
  '.gif':  'image/gif',
  '.svg':  'image/svg+xml',
  '.ico':  'image/x-icon',
  '.webp': 'image/webp',
  '.woff': 'font/woff',
  '.woff2':'font/woff2',
  '.ttf':  'font/ttf',
  '.otf':  'font/otf',
  '.mp3':  'audio/mpeg',
  '.wav':  'audio/wav',
  '.ogg':  'audio/ogg',
  '.mp4':  'video/mp4',
  '.webm': 'video/webm',
  '.txt':  'text/plain; charset=utf-8',
  '.xml':  'text/xml; charset=utf-8',
  '.data': 'application/octet-stream',
  '.bin':  'application/octet-stream',
};

// Don't compress already-compressed or tiny formats
var NO_COMPRESS = new Set(['.wasm', '.png', '.jpg', '.jpeg', '.gif', '.webp',
  '.woff', '.woff2', '.mp3', '.ogg', '.mp4', '.webm', '.gz', '.zip', '.br']);

function serve(req, res) {
  var url = new URL(req.url, 'http://localhost');
  var path = decodeURIComponent(url.pathname);
  if (path === '/') path = '/index.html';

  // Block path traversal
  if (path.includes('..')) {
    res.writeHead(400);
    res.end('Bad request');
    return;
  }

  var filePath = join(root, path);
  var stat;
  try {
    stat = statSync(filePath);
    if (stat.isDirectory()) {
      filePath = join(filePath, 'index.html');
      stat = statSync(filePath);
    }
  } catch {
    res.writeHead(404, { 'Content-Type': 'text/plain' });
    res.end('Not found: ' + path);
    return;
  }

  var ext = extname(filePath).toLowerCase();
  var contentType = MIME[ext] || 'application/octet-stream';
  var size = stat.size;

  // Common headers
  var headers = {
    'Content-Type': contentType,
    'Cross-Origin-Opener-Policy': 'same-origin',
    'Cross-Origin-Embedder-Policy': 'credentialless',
    'Cache-Control': 'no-cache',
    'Accept-Ranges': 'bytes',
  };

  // Range requests (for large .wasm / media files)
  var range = req.headers.range;
  if (range) {
    var match = range.match(/bytes=(\d+)-(\d*)/);
    if (match) {
      var start = parseInt(match[1]);
      var end = match[2] ? parseInt(match[2]) : size - 1;
      if (start >= size || end >= size) {
        res.writeHead(416, { 'Content-Range': 'bytes */' + size });
        res.end();
        return;
      }
      headers['Content-Range'] = 'bytes ' + start + '-' + end + '/' + size;
      headers['Content-Length'] = (end - start + 1).toString();
      res.writeHead(206, headers);
      createReadStream(filePath, { start, end }).pipe(res);
      return;
    }
  }

  // Compression for text-based content
  var acceptEncoding = req.headers['accept-encoding'] || '';
  if (!NO_COMPRESS.has(ext) && size > 1024) {
    if (acceptEncoding.includes('gzip')) {
      headers['Content-Encoding'] = 'gzip';
      res.writeHead(200, headers);
      createReadStream(filePath).pipe(createGzip()).pipe(res);
      return;
    }
    if (acceptEncoding.includes('deflate')) {
      headers['Content-Encoding'] = 'deflate';
      res.writeHead(200, headers);
      createReadStream(filePath).pipe(createDeflate()).pipe(res);
      return;
    }
  }

  // No compression
  headers['Content-Length'] = size.toString();
  res.writeHead(200, headers);
  createReadStream(filePath).pipe(res);
}

var server = createServer(serve);
server.listen(port, function() {
  console.log('\x1b[36m  Serving\x1b[0m  ' + root);
  console.log('\x1b[36m  URL\x1b[0m      http://localhost:' + port + '/');
  console.log('\x1b[36m  Headers\x1b[0m  COOP: same-origin, COEP: credentialless');
  console.log('\x1b[2m  Press Ctrl+C to stop\x1b[0m');
});
