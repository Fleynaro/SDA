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
    MuiCssBaseline: {
      styleOverrides: {
        body: {
          userSelect: 'none',
          ul: {
            listStyle: 'none',
          },
          '*::-webkit-scrollbar': {
            width: '8px',
            height: '8px',
          },
          '*::-webkit-scrollbar-track': {
            backgroundColor: 'transparent',
          },
          '*::-webkit-scrollbar-thumb': {
            backgroundColor: '#3d3d3d',
            borderRadius: '4px',
          },
          '*::-webkit-scrollbar-thumb:hover': {
            backgroundColor: '#424242',
          },
        },
      },
    },
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
