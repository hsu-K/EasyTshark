const { contextBridge, ipcRenderer, app } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  openFileDialog: () => ipcRenderer.invoke('open-file-dialog'),
  showSavePath: () => ipcRenderer.invoke('show-save-dialog'),
})