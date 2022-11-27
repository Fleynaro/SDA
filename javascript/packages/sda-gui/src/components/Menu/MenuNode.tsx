import { ReactNode, useEffect, useState } from 'react';
import {
  MenuItem,
  ListItemIcon,
  Typography,
  ListItemText,
  Popper,
  Paper,
  MenuList,
} from '@mui/material';

export interface MenuNodeProps {
  label?: string;
  icon?: ReactNode;
  hotkey?: string;
  onClick?: () => void;
  disabled?: boolean;
  children?: ReactNode;
}

export const MenuNode = ({ label, icon, hotkey, onClick, disabled, children }: MenuNodeProps) => {
  const [opened, setOpened] = useState(false);
  const [requestClosed, setRequestClosed] = useState(false);
  const [requestOpened, setRequestOpened] = useState(false);
  const [anchorEl, setAnchorEl] = useState<HTMLElement>();

  useEffect(() => {
    if (requestClosed && !requestOpened) {
      setOpened(false);
      setRequestClosed(false);
    }
  }, [requestClosed, requestOpened]);

  return (
    <>
      {label ? (
        <MenuItem
          onClick={() => {
            onClick?.();
          }}
          onMouseEnter={(e) => {
            if (children) {
              setOpened(true);
              setAnchorEl(e.currentTarget);
            }
          }}
          onMouseLeave={() => {
            setTimeout(() => setRequestClosed(true));
          }}
          disabled={disabled}
        >
          {icon && <ListItemIcon>{icon}</ListItemIcon>}
          <ListItemText>{label}</ListItemText>
          {children ? (
            <Typography variant="body2" color="text.secondary">
              {'>'}
            </Typography>
          ) : (
            hotkey && (
              <Typography variant="body2" color="text.secondary">
                {hotkey}
              </Typography>
            )
          )}
        </MenuItem>
      ) : (
        <Paper sx={{ width: 200, maxWidth: '100%' }}>
          <MenuList>{children}</MenuList>
        </Paper>
      )}
      {children && opened && (
        <Popper
          open={opened}
          anchorEl={anchorEl}
          placement="right-start"
          onMouseEnter={() => setRequestOpened(true)}
          onMouseLeave={() => setRequestOpened(false)}
        >
          <Paper sx={{ width: 200, maxWidth: '100%' }}>
            <MenuList>{children}</MenuList>
          </Paper>
        </Popper>
      )}
    </>
  );
};
