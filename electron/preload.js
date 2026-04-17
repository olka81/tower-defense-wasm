'use strict';

const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  isElectron: true,

  // Saves the JSON string returned by Module.game_save_state().
  // Opens a native Save dialog; resolves to { success, filePath? }.
  saveState: (jsonData) => ipcRenderer.invoke('save-state', jsonData),

  // Opens a native Open dialog and returns the file contents.
  // Resolves to { success, data? } where data is the raw JSON string.
  loadState: () => ipcRenderer.invoke('load-state'),
});
