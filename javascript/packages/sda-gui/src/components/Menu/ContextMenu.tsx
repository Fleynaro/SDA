import { useState, useCallback, ReactNode } from 'react';
import { PopperPlacementType, PopoverPosition, Popper, ClickAwayListener } from '@mui/material';
import { MenuNode } from './MenuNode';

export interface ContextMenuProps {
  opened: boolean;
  placement?: PopperPlacementType;
  anchorEl?: HTMLElement;
  anchorPosition?: PopoverPosition;
  onClose: () => void;
  children?: ReactNode;
}

export interface ContextMenuHook {
  open: (event: React.MouseEvent<HTMLElement>) => void;
  openAtPos: (x: number, y: number) => void;
  close: () => void;
  props: ContextMenuProps;
}

export const useContextMenu = (mousePos = true): ContextMenuHook => {
  const [opened, setOpened] = useState(false);
  const [anchorEl, setAnchorEl] = useState<HTMLElement>();
  const [anchorPosition, setAnchorPosition] = useState<PopoverPosition>();

  const open = useCallback(
    (event: React.MouseEvent<HTMLElement>) => {
      setOpened(true);
      if (mousePos) {
        setAnchorPosition({
          top: event.clientY,
          left: event.clientX,
        });
      } else {
        setAnchorEl(event.currentTarget);
      }
    },
    [mousePos],
  );

  const openAtPos = useCallback(
    (x: number, y: number) => {
      setOpened(true);
      setAnchorPosition({
        top: y,
        left: x,
      });
    },
    [mousePos],
  );

  const close = useCallback(() => {
    setOpened(false);
  }, []);

  const props: ContextMenuProps = {
    opened,
    anchorPosition,
    anchorEl,
    onClose: close,
  };

  return {
    open,
    openAtPos,
    close,
    props,
  };
};

export const ContextMenu = (props: ContextMenuProps) => {
  return (
    <>
      {props.opened && (
        <ClickAwayListener onClickAway={props.onClose}>
          <Popper
            open={props.opened}
            placement={props.placement}
            anchorEl={props.anchorEl}
            onClick={props.onClose}
            style={{
              top: props.anchorPosition?.top,
              left: props.anchorPosition?.left,
            }}
          >
            <MenuNode>{props.children}</MenuNode>
          </Popper>
        </ClickAwayListener>
      )}
    </>
  );
};
