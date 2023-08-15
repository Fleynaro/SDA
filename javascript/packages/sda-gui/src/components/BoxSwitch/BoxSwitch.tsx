import { Box, SxProps, Theme } from '@mui/material';
import { createContext, useContext } from 'react';

const BoxSwitchContext = createContext<string | undefined>(undefined);

export interface BoxSwitchProps {
  value?: string;
  children?: React.ReactNode;
}

export function BoxSwitch({ value, children }: BoxSwitchProps) {
  return <BoxSwitchContext.Provider value={value}>{children}</BoxSwitchContext.Provider>;
}

export interface BoxSwitchCaseProps {
  value: string;
  children?: React.ReactNode;
  sx?: SxProps<Theme>;
}

export function BoxSwitchCase({ value, children, sx }: BoxSwitchCaseProps) {
  const context = useContext(BoxSwitchContext);
  return (
    <Box
      sx={{ height: '100%', ...sx, ...(context !== value ? { display: 'none' } : {}) }}
      aria-label="box-switch-case"
    >
      {children}
    </Box>
  );
}
