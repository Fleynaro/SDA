import { ReactNode, useState, createContext, useCallback, useContext, useEffect } from 'react';
import { Button, Box, emphasize, Theme, Popper, ClickAwayListener } from '@mui/material';
import { makeStyles } from '@mui/styles';
import { MenuNode } from './MenuNode';

const useStyles = makeStyles((theme: Theme) => ({
  menu: {
    display: 'flex',
    alignItems: 'center',
    backgroundColor: emphasize(theme.palette.background.default, 0.1),
    height: '20px',
    width: '100%',
  },
  menuTopBtn: {
    backgroundColor: emphasize(theme.palette.background.default, 0.1),
    color: 'white',
    height: '20px',
    borderRadius: 0,
    textTransform: 'none',
    cursor: 'default',
    '&:hover': {
      backgroundColor: emphasize(theme.palette.background.default, 0.2),
    },
  },
}));

export interface MenuBarContextValue {
  opened: string;
  setOpened: (opened: string) => void;
}

const MenuBarContext = createContext<MenuBarContextValue | null>(null);

export interface MenuBarItemProps {
  label: string;
  children?: ReactNode;
}

export const MenuBarItem = ({ label, children }: MenuBarItemProps) => {
  const classes = useStyles();
  const menuBarCtx = useContext(MenuBarContext);
  const [anchorEl, setAnchorEl] = useState<HTMLElement>();
  const isOpened = menuBarCtx?.opened === label;

  const openMenu = useCallback((anchorEl: HTMLElement) => {
    setAnchorEl(anchorEl);
    menuBarCtx?.setOpened(label);
  }, []);

  const closeMenu = useCallback(() => {
    menuBarCtx?.setOpened('');
  }, []);

  return (
    <>
      <Button
        className={classes.menuTopBtn}
        variant="contained"
        onClick={(e) => {
          openMenu(e.currentTarget);
        }}
        onMouseEnter={(e) => {
          if (menuBarCtx?.opened) {
            openMenu(e.currentTarget);
          }
        }}
      >
        {label}
      </Button>
      {isOpened && (
        <ClickAwayListener onClickAway={closeMenu}>
          <Popper open={isOpened} anchorEl={anchorEl} onClick={closeMenu} placement="bottom-start">
            <MenuNode>{children}</MenuNode>
          </Popper>
        </ClickAwayListener>
      )}
    </>
  );
};

export interface MenuBarProps {
  onMenuOpen?: () => void;
  children?: ReactNode;
}

export const MenuBar = ({ onMenuOpen, children }: MenuBarProps) => {
  const classes = useStyles();
  const [opened, setOpened] = useState('');
  useEffect(() => {
    if (opened) {
      onMenuOpen?.();
    }
  }, [opened]);
  return (
    <>
      <Box className={classes.menu}>
        <MenuBarContext.Provider value={{ opened, setOpened }}>{children}</MenuBarContext.Provider>
      </Box>
    </>
  );
};
