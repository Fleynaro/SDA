import { Box, SxProps, Theme } from '@mui/material';
import { createContext, useContext } from 'react';

const BoxSwitchContext = createContext<string | undefined>(undefined);

export interface BoxSwitchProps {
  value?: string;
  children?: React.ReactNode;
  sx?: SxProps<Theme>;
}

export function BoxSwitch({ value, children, sx }: BoxSwitchProps) {
  return (
    <BoxSwitchContext.Provider value={value}>
      <Box sx={sx}>{children}</Box>
    </BoxSwitchContext.Provider>
  );
}

export interface BoxSwitchCaseProps {
  value: string;
  children?: React.ReactNode;
  sx?: SxProps<Theme>;
}

export function BoxSwitchCase({ value, children, sx }: BoxSwitchCaseProps) {
  const context = useContext(BoxSwitchContext);
  return (
    <Box sx={{ height: '100%', ...sx, ...(context !== value ? { display: 'none' } : {}) }}>
      {children}
    </Box>
  );
}
