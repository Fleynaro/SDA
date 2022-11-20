import { ReactNode } from 'react';
import { ButtonGroup, Button } from '@mui/material';
import { useContextMenu, ContextMenu } from 'components/ContextMenu';
export {
  ContextMenuList as MenuBarList,
  ContextMenuItem as MenuBarItem,
} from 'components/ContextMenu';

export interface MenuBarTopItemProps {
  label: string;
  children?: ReactNode;
}

export const MenuBarTopItem = ({ label, children }: MenuBarTopItemProps) => {
  const contextMenu = useContextMenu();
  return (
    <>
      <Button onClick={(e) => contextMenu.open(e)}>{label}</Button>
      <ContextMenu {...contextMenu.props}>{children}</ContextMenu>
    </>
  );
};

export interface MenuBarProps {
  children?: ReactNode;
}

export const MenuBar = ({ children }: MenuBarProps) => {
  return (
    <ButtonGroup variant="text" sx={{ height: 20 }}>
      {children}
    </ButtonGroup>
  );
};
