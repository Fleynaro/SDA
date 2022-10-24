import { useState, useCallback, useImperativeHandle, forwardRef, ReactNode } from 'react';
import {
  Button,
  Dialog as DialogMui,
  DialogActions,
  DialogContent,
  DialogTitle,
} from '@mui/material';

export interface DialogProps {
  title: string;
  showCancelButton?: boolean;
  onClose?: () => void;
}

export interface DialogRef {
  open: (body: ReactNode, actions?: ReactNode) => void;
}

export const Dialog = forwardRef((props: DialogProps, ref: React.Ref<DialogRef>) => {
  const [opened, setOpened] = useState(false);
  const [body, setBody] = useState<ReactNode>();
  const [actions, setActions] = useState<ReactNode>();

  useImperativeHandle(ref, () => ({
    open: (body: ReactNode, actions?: ReactNode) => {
      setOpened(true);
      setBody(body);
      setActions(actions);
    },
  }));

  const onClose = useCallback(() => {
    setOpened(false);
    if (props.onClose) {
      props.onClose();
    }
  }, [props]);

  return (
    <DialogMui open={opened} onClose={onClose} fullWidth maxWidth="xs">
      {props.title && <DialogTitle>{props.title}</DialogTitle>}

      <DialogContent>{body}</DialogContent>

      <DialogActions>
        {actions}
        <Button onClick={onClose}>{props.showCancelButton ? 'Cancel' : 'Close'}</Button>
      </DialogActions>
    </DialogMui>
  );
});
