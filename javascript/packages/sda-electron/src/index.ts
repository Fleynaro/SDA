import { app } from 'electron';
import { windowController } from './controllers';
import sdaApp from './app';

app.whenReady().then(async () => {
  sdaApp.init();
  windowController.openProjectManagerWindow();

  // app.on('activate', () => {
  //     if (BrowserWindow.getAllWindows().length === 0)
  //         createWindow();
  // })
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});
