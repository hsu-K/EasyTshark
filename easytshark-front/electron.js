const { app, BrowserWindow, electron, Menu, ipcMain, dialog } = require('electron');
const path = require('path');
const fs = require('fs');
const dayjs = require('dayjs');


const windows = {
  windowIndex: 0,
  mainWindow: null,
  subWindow: null,
}

function createWindow() {
  console.log(path.join(__dirname, 'preload.js'));
  windows.mainWindow = new BrowserWindow({
    width: 1600,
    height: 1100,
    title: 'MyTshark',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false
    }
  });

  // 隱藏electron菜單欄
  Menu.setApplicationMenu(null);

  const mode = process.argv[2];
  if (mode === 'dev') {
    windows.mainWindow.loadURL('http://localhost:3000/#/data/dataPacket/all');
    windows.mainWindow.webContents.openDevTools();
  } else {
    windows.mainWindow.loadURL(`file://${path.join(__dirname, 'build/index.html')}#/home`);
    windows.mainWindow.webContents.openDevTools();
  }
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});

app.on('web-contents-created', (event, webContents) => {
  webContents.setWindowOpenHandler((details) => {
    const { url } = details;

    const params = url.split('?')[1];
    const browserWindow = new BrowserWindow({
      width: 1300,
      height: 900,
      title: 'MyTshark'
    });

    if (windows.windowIndex === 0) {
      windows.subWindow = browserWindow;
    }
    else {
      windows[`subWindow${windows.windowIndex}`] = browserWindow;
    }

    Menu.setApplicationMenu(null);
    const mode = process.argv[2];
    if (mode === 'dev') {
      browserWindow.loadURL(`http://localhost:3000/#/detail?${params}`);
      browserWindow.webContents.openDevTools();
    } else {
      browserWindow.loadURL(`file://${path.join(__dirname, 'build/index.html')}#/detail?${params}`,);
      // browserWindow.webContents.openDevTools();
    }

    windows.windowIndex += 1;

    return { action: 'deny' };

  });

  // webContents.on('did-create-window', (childWindow) => {
  //   childWindow.webContents.openDevTools({ mode: 'detach' });
  // });
});


ipcMain.handle('open-file-dialog', async (event) => {
  const result = await dialog.showOpenDialog(windows.mainWindow, {
    properties: ['openFile'],
    filters: [
      { name: 'pcap', extensions: ['pcap', 'cap', 'pcapng'] },
    ],
  });

  if (!result.canceled && result.filePaths.length > 0) {
    return result.filePaths[0];
  }

  return null;

});


ipcMain.handle('show-save-dialog', async (event) => {
  const result = await dialog.showSaveDialog(windows.mainWindow, {
    title: "保存文件",
    defaultPath: `easytshark_${dayjs().format('YYYY-MM-DD')}_${dayjs().format('HH-mm-ss')}.pcap`,
    buttonLabel: "保存",
    filters: [
      { name: '所有文件', extensions: ['*'] },
    ]
  });
  if (!result.canceled) {
    return result.filePath;
  }
  else {
    return null;
  }
});