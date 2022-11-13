import { useState, useCallback, ReactNode } from 'react';
import {
  Popover,
  MenuList,
  MenuListProps,
  MenuItem,
  MenuItemProps,
  PopoverPosition,
} from '@mui/material';

export interface ContextMenuProps {
  opened: boolean;
  anchorPosition: PopoverPosition | undefined;
  onClose: () => void;
  children?: ReactNode;
}

export interface ContextMenuHook {
  open: (event: React.MouseEvent<HTMLElement>) => void;
  close: () => void;
  props: ContextMenuProps;
}

export const useContextMenu = (): ContextMenuHook => {
  const [opened, setOpened] = useState(false);
  const [body, setBody] = useState<ReactNode>();
  const [anchorPosition, setAnchorPosition] = useState<PopoverPosition>();

  const open = useCallback((event: React.MouseEvent<HTMLElement>) => {
    setOpened(true);
    setBody(body);
    setAnchorPosition({
      top: event.clientY,
      left: event.clientX,
    });
  }, []);

  const close = useCallback(() => {
    setOpened(false);
  }, []);

  const props: ContextMenuProps = {
    opened,
    anchorPosition,
    onClose: close,
  };

  return {
    open,
    close,
    props,
  };
};

export const ContextMenu = (props: ContextMenuProps) => {
  return (
    <Popover
      open={props.opened}
      anchorReference="anchorPosition"
      anchorPosition={props.anchorPosition}
      onClose={props.onClose}
      onClick={props.onClose}
    >
      {props.children}
    </Popover>
  );
};

export const ContextMenuList = ({ children, ...props }: MenuListProps) => {
  return <MenuList {...props}>{children}</MenuList>;
};

export const ContextMenuItem = ({ children, ...props }: MenuItemProps) => {
  return <MenuItem {...props}>{children}</MenuItem>;
};
