'use strict';

const { app, BrowserWindow, ipcMain, dialog, protocol } = require('electron');
const path = require('path');
const fs   = require('fs');

const WEB_DIR = path.join(__dirname, '..', 'web');

const MIME = {
  '.html': 'text/html; charset=utf-8',
  '.js':   'application/javascript; charset=utf-8',
  '.css':  'text/css; charset=utf-8',
  '.wasm': 'application/wasm',
};

// Must be called before app is ready.
// A privileged scheme lets fetch() + WebAssembly.instantiateStreaming() work
// without the file:// CORS restrictions that block WASM loading.
protocol.registerSchemesAsPrivileged([{
  scheme: 'app',
  privileges: { standard: true, secure: true, supportFetchAPI: true },
}]);

function createWindow() {
  const win = new BrowserWindow({
    width:  940,
    height: 720,
    title:  'Tower Defense',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
    },
  });

  win.loadURL('app://localhost/index.html');
}

// ---- Custom app:// file server ---------------------------------------------
function handleAppProtocol(request) {
  const { pathname } = new URL(request.url);
  const rel      = pathname.replace(/^\//, '') || 'index.html';
  const filePath = path.join(WEB_DIR, rel);

  // Prevent path-traversal outside web/
  if (!filePath.startsWith(WEB_DIR)) {
    return new Response('Forbidden', { status: 403 });
  }

  let data;
  try {
    data = fs.readFileSync(filePath);
  } catch {
    return new Response(`Not found: ${pathname}`, { status: 404 });
  }

  const ext         = path.extname(filePath).toLowerCase();
  const contentType = MIME[ext] || 'application/octet-stream';
  return new Response(data, { headers: { 'Content-Type': contentType } });
}

// ---- IPC: save game state to a user-chosen file ----------------------------
ipcMain.handle('save-state', async (_event, jsonData) => {
  const { canceled, filePath } = await dialog.showSaveDialog({
    title:       'Save Game',
    defaultPath: 'tower-defense-save.json',
    filters:     [{ name: 'JSON Save File', extensions: ['json'] }],
  });
  if (canceled || !filePath) return { success: false };
  fs.writeFileSync(filePath, jsonData, 'utf-8');
  return { success: true, filePath };
});

// ---- IPC: load game state from a user-chosen file --------------------------
ipcMain.handle('load-state', async () => {
  const { canceled, filePaths } = await dialog.showOpenDialog({
    title:      'Load Game',
    filters:    [{ name: 'JSON Save File', extensions: ['json'] }],
    properties: ['openFile'],
  });
  if (canceled || filePaths.length === 0) return { success: false };
  try {
    const data = fs.readFileSync(filePaths[0], 'utf-8');
    return { success: true, data };
  } catch (err) {
    return { success: false, error: err.message };
  }
});

// ---- App lifecycle ---------------------------------------------------------
app.whenReady().then(() => {
  protocol.handle('app', handleAppProtocol);
  createWindow();
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
