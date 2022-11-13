import { useState, useCallback, useImperativeHandle, forwardRef, ReactNode } from 'react';
import {
  Popover,
  MenuList,
  MenuListProps,
  MenuItem,
  MenuItemProps,
  PopoverPosition,
} from '@mui/material';

export interface ContextMenuRef {
  open: (event: React.MouseEvent<HTMLElement>, body: ReactNode) => void;
  close: () => void;
}

export const ContextMenu = forwardRef((_props: unknown, ref: React.Ref<ContextMenuRef>) => {
  const [opened, setOpened] = useState(false);
  const [body, setBody] = useState<ReactNode>();
  const [anchorPosition, setAnchorPosition] = useState<PopoverPosition>();

  useImperativeHandle(ref, () => ({
    open: (event: React.MouseEvent<HTMLElement>, body: ReactNode) => {
      setOpened(true);
      setBody(body);
      setAnchorPosition({
        top: event.clientY,
        left: event.clientX,
      });
    },

    close: () => {
      onClose();
    },
  }));

  const onClose = useCallback(() => {
    setOpened(false);
  }, []);

  return (
    <Popover
      open={opened}
      anchorReference="anchorPosition"
      anchorPosition={anchorPosition}
      onClose={onClose}
    >
      {body}
    </Popover>
  );
});

export const ContextMenuList = ({ children, ...props }: MenuListProps) => {
  return <MenuList {...props}>{children}</MenuList>;
};

export const ContextMenuItem = ({ children, ...props }: MenuItemProps) => {
  return <MenuItem {...props}>{children}</MenuItem>;
};
