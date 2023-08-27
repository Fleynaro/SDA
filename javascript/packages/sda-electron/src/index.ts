import { app, session } from 'electron';
import isElectronDev from 'electron-is-dev';
import { windowController } from './controllers';
import sdaApp from './app';

app.whenReady().then(async () => {
  sdaApp.init();
  windowController.openProjectManagerWindow();

  // install useful chrome extensions
  if (isElectronDev) {
    // https://github.com/facebook/react/issues/25843#issuecomment-1406766561
    // (download at SDA\javascript\packages\sda-electron\data\chrome-extension\ReactDevTools.zip)
    await session.defaultSession.loadExtension('C:\\Users\\Fleynaro\\Downloads\\ReactDevTools', {
      allowFileAccess: true,
    });
  }

  // app.on('activate', () => {
  //     if (BrowserWindow.getAllWindows().length === 0)
  //         createWindow();
  // })
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});
