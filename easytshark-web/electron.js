const { app, BrowserWindow, electron, Menu, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');
const dayjs = require('dayjs');

const windows = {
  windowIndex: 0,
  mainWindow: null,
  subWindow: null,
}


function createWindow() {
  windows.mainWindow = new BrowserWindow({
    width: 1200,
    height: 900,
    title: 'MyTshark',
  });


  Menu.setApplicationMenu(null);

  const mode = process.argv[2];
  if (mode === 'dev') {
    windows.mainWindow.loadURL('http://localhost:3000/#/data/dataPacket/all');
    windows.mainWindow.webContents.openDevTools();
  } else {
    windows.mainWindow.loadURL(`file://${path.join(__dirname, 'build/index.html')}#/home`);
  }
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});

app.on('web-contents-created', function (event, webContents) {

  // 監聽window.open事件
  webContents.setWindowOpenHandler((details) => {

    // 獲取要打開的URL
    const { url } = details;
    const params = url.split('?')[1];
    const browserWindow = new BrowserWindow({
      width: 1200,
      height: 800,
      title: 'MyTshark',
    });

    if (windows.windowIndex === 0) {
      windows.subWindow = browserWindow;
    }
    else{
      windows[`subWindow${windows.windowIndex}`] = browserWindow;
    }

    Menu.setApplicationMenu(null);

    const mode = process.argv[2];
    if(mode === 'dev'){
      browserWindow.loadURL(`http://localhost:3000/#/detail?${params}`);
      browserWindow.webContents.openDevTools();
    }else{
      browserWindow.loadURL(`file://${path.join(__dirname, 'build/index.html')}#/detail?${params}`, );
    }

    windows.windowIndex = windows.windowIndex + 1;

    return {action: 'deny'};

  });
})

ipcMain.handle('open-file-dialog', async (event) => {
  const result = await dialog.showOpenDialog(windows.mainWindow, {
    properties: ['openFile'],
    filters: [
      { name: 'pcap', extensions: ['pcap', 'cap', 'pcapng']},
    ],
  });

  if (!result.canceled && result.filePaths.length > 0){
    return result.filePaths[0];
  }

  return null; // 用戶取消了選擇
});

ipcMain.handle('show-save-dialog', async (event) => {
  const result = await dialog.showSaveDialog(windows.mainWindow, {
    title: 'Save File',
    defaultPath: `easytshark_${dayjs().format('YYYY-MM-DD')}_${dayjs().format('HH-mm-ss')}.pcap`,
    buttonLabel: 'Save',
    filters: [
      { name: 'all Files', extensions: ['*']},
    ],
  });

  if (!result.canceled) {
    return result.filePath;
  }
  else{
    return null; // 用戶取消了選擇
  }

});