const { app, BrowserWindow, electron, Menu } = require('electron');
const path = require('path');

function createWindow() {
  const window = new BrowserWindow({
    width: 1200,
    height: 900,
    title: 'MyTshark',
  });

  Menu.setApplicationMenu(null);

  const mode = process.argv[2];
  if (mode === 'dev') {
    window.loadURL('http://localhost:3000/#/data/dataPacket/all');
    window.webContents.openDevTools();
  } else {
    window.loadURL(`file://${path.join(__dirname, 'build/index.html')}#/home`);
  }
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});