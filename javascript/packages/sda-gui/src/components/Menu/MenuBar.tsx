import { ReactNode, useState, createContext, useCallback, useContext } from 'react';
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
  opened: boolean;
  openMenu: (menu: ReactNode, anchorEl: HTMLElement) => void;
}

const MenuBarContext = createContext<MenuBarContextValue | null>(null);

export interface MenuBarItemProps {
  label: string;
  children?: ReactNode;
}

export const MenuBarItem = ({ label, children }: MenuBarItemProps) => {
  const classes = useStyles();
  const menuBarCtx = useContext(MenuBarContext);
  return (
    <>
      <Button
        className={classes.menuTopBtn}
        variant="contained"
        onClick={(e) => {
          menuBarCtx?.openMenu(children, e.currentTarget);
        }}
        onMouseEnter={(e) => {
          if (menuBarCtx?.opened) {
            menuBarCtx?.openMenu(children, e.currentTarget);
          }
        }}
      >
        {label}
      </Button>
    </>
  );
};

export interface MenuBarProps {
  children?: ReactNode;
}

export const MenuBar = ({ children }: MenuBarProps) => {
  const classes = useStyles();
  const [opened, setOpened] = useState(false);
  const [menu, setMenu] = useState<ReactNode>();
  const [anchorEl, setAnchorEl] = useState<HTMLElement>();

  const openMenu = useCallback((menu: ReactNode, anchorEl: HTMLElement) => {
    setOpened(true);
    setAnchorEl(anchorEl);
    setMenu(menu);
  }, []);

  const closeMenu = useCallback(() => {
    setOpened(false);
  }, []);

  return (
    <>
      <Box className={classes.menu}>
        <MenuBarContext.Provider value={{ opened, openMenu }}>{children}</MenuBarContext.Provider>
      </Box>
      {children && opened && (
        <ClickAwayListener onClickAway={closeMenu}>
          <Popper open={opened} anchorEl={anchorEl} placement="bottom-start">
            <MenuNode>{menu}</MenuNode>
          </Popper>
        </ClickAwayListener>
      )}
    </>
  );
};
