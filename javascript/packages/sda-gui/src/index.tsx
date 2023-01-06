import React from 'react';
import ReactDOM from 'react-dom/client';
import { CssBaseline, ThemeProvider } from '@mui/material';
import { setWindow } from 'sda-electron/api/common';
import theme from './theme';
import App from './App';
import { CrashProvider } from 'providers/CrashProvider';

setWindow(window);

const root = ReactDOM.createRoot(document.getElementById('root') as HTMLElement);

// warning: React.StrictMode causes double rendering of components (but in dev mode only)
root.render(
  <React.StrictMode>
    <ThemeProvider theme={theme}>
      <CrashProvider>
        <CssBaseline />
        <App />
      </CrashProvider>
    </ThemeProvider>
  </React.StrictMode>,
);
