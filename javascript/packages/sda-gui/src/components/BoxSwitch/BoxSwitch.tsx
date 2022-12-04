import { Box } from '@mui/material';
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
}

export function BoxSwitchCase({ value, children }: BoxSwitchCaseProps) {
  const context = useContext(BoxSwitchContext);
  return <Box hidden={context !== value}>{children}</Box>;
}
