import { createTheme } from '@mui/material';

export default createTheme({
  palette: {
    mode: 'dark',
  },

  spacing: 2,

  typography: {
    fontSize: 11,
  },

  components: {
    MuiInputBase: {
      styleOverrides: {
        root: {
          height: '40px',
        },
      },
    },
  },
});
