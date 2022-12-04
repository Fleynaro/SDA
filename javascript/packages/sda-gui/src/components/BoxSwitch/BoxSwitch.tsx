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
}

export function BoxSwitchCase({ value, children }: BoxSwitchCaseProps) {
  const context = useContext(BoxSwitchContext);
  return (
    <Box hidden={context !== value} sx={{ height: '100%' }}>
      {children}
    </Box>
  );
}
