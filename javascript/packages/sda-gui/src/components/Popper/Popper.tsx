import {
  useState,
  useCallback,
  ReactNode,
  createContext,
  useContext,
  useRef,
  MutableRefObject,
  useEffect,
} from 'react';
import {
  PopperPlacementType,
  PopoverPosition,
  Popper as MuiPopper,
  ClickAwayListener,
} from '@mui/material';
import { noop } from 'utils';

export interface PopperProps {
  opened: boolean;
  hovered: MutableRefObject<boolean>;
  placement?: PopperPlacementType;
  anchorEl?: HTMLElement;
  anchorPosition?: PopoverPosition;
  onClose: () => void;
  withTimer: (callback: () => void, timeout: number) => void;
  closeOnMouseLeave?: boolean;
  closeOnContentClick?: boolean;
  children?: ReactNode;
}

export interface PopperHook {
  open: (event: React.MouseEvent<HTMLElement>, mousePos?: boolean) => void;
  openAtPos: (x: number, y: number) => void;
  setContent: (body: ReactNode) => void;
  withTimer: (callback: () => void, timeout: number) => void;
  close: () => void;
  setCloseCallback: (callback: () => void) => void;
  props: PopperProps;
}

const PopperContext = createContext<PopperHook | null>(null);
export const PopperContextProvider = PopperContext.Provider;

export const usePopperFromContext = () => {
  const ctx = useContext(PopperContext);
  if (!ctx) throw new Error('PopperContext is not set');
  return ctx;
};

export const usePopper = (): PopperHook => {
  const [opened, setOpened] = useState(false);
  const [anchorEl, setAnchorEl] = useState<HTMLElement>();
  const [anchorPosition, setAnchorPosition] = useState<PopoverPosition>();
  const [body, setContent] = useState<ReactNode | undefined>(undefined);
  const hovered = useRef(false);
  const closeCallback = useRef(noop);
  const timer = useRef<NodeJS.Timeout>();

  const callCloseCallback = useCallback(() => {
    closeCallback.current();
    closeCallback.current = noop;
  }, [closeCallback]);

  const open = useCallback(
    (event: React.MouseEvent<HTMLElement>, mousePos = true) => {
      setOpened(true);
      if (mousePos) {
        setAnchorPosition({
          top: event.clientY,
          left: event.clientX,
        });
      } else {
        setAnchorEl(event.currentTarget);
      }
      callCloseCallback();
    },
    [callCloseCallback],
  );

  const openAtPos = useCallback(
    (x: number, y: number) => {
      setOpened(true);
      setAnchorPosition({
        top: y,
        left: x,
      });
      callCloseCallback();
    },
    [callCloseCallback],
  );

  const withTimer = useCallback(
    (callback: () => void, timeout: number) => {
      clearTimeout(timer.current);
      timer.current = setTimeout(() => {
        callback();
      }, timeout);
    },
    [timer],
  );

  const close = useCallback(() => {
    if (hovered.current) return;
    setOpened(false);
    callCloseCallback();
  }, [callCloseCallback]);

  const setCloseCallback = useCallback((callback: () => void) => {
    closeCallback.current = callback;
  }, []);

  const props: PopperProps = {
    opened,
    hovered,
    anchorPosition,
    anchorEl,
    onClose: close,
    withTimer,
    children: body,
  };

  return {
    open,
    openAtPos,
    setContent,
    withTimer,
    close,
    setCloseCallback,
    props,
  };
};

export const Popper = (props: PopperProps) => {
  return (
    <>
      {props.opened && (
        <ClickAwayListener onClickAway={props.onClose} mouseEvent="onMouseUp">
          <MuiPopper
            open={props.opened}
            placement={props.placement}
            anchorEl={props.anchorEl}
            style={{
              top: props.anchorPosition?.top,
              left: props.anchorPosition?.left,
            }}
            onClick={() => {
              if (props.closeOnContentClick) {
                props.hovered.current = false;
                props.onClose();
              }
            }}
            onMouseEnter={() => {
              props.hovered.current = true;
              props.withTimer(noop, 0);
            }}
            onMouseLeave={() => {
              props.hovered.current = false;
              if (props.closeOnMouseLeave) {
                props.withTimer(props.onClose, 300);
              }
            }}
          >
            {props.children}
          </MuiPopper>
        </ClickAwayListener>
      )}
    </>
  );
};
