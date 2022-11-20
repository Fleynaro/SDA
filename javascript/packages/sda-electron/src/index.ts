import { app } from 'electron';
import { windowController } from './controllers';
import { initApp } from './app';

app.whenReady().then(async () => {
  initApp();
  windowController.openProjectManagerWindow();

  // app.on('activate', () => {
  //     if (BrowserWindow.getAllWindows().length === 0)
  //         createWindow();
  // })
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});
