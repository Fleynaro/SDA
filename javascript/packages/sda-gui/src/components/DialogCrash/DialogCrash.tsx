import { useCallback, useEffect, useRef } from 'react';
import { TextareaAutosize } from '@mui/material';
import { Dialog, DialogRef } from '../Dialog';
import { useCrash } from 'providers/CrashProvider';

const DialogCrash = () => {
  const { error, setError } = useCrash();
  const errorDialogRef = useRef<DialogRef>(null);

  useEffect(() => {
    if (!error) return;
    if (errorDialogRef.current) {
      errorDialogRef.current.open(
        <>
          <p>{error.message}</p>
          <TextareaAutosize value={error.stack} minRows={3} style={{ width: '100%' }} />
        </>,
      );
    }
  }, [error]);

  const onClose = useCallback(() => {
    setError(undefined);
  }, [setError]);

  return <Dialog title="Error occured" ref={errorDialogRef} onClose={onClose} />;
};

export default DialogCrash;
