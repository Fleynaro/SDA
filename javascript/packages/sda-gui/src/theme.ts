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
    MuiTabs: {
      styleOverrides: {
        root: {
          height: '30px',
          minHeight: '30px',
        },
      },
    },
    MuiTab: {
      styleOverrides: {
        root: {
          // https://github.com/mui/material-ui/issues/5391#issuecomment-493754603
          minHeight: 'auto',
          padding: 10,
        },
      },
    },
  },
});
