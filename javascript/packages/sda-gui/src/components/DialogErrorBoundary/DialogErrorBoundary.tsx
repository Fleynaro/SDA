import { useEffect, useRef } from 'react';
import { ErrorBoundary, FallbackProps } from 'react-error-boundary';
import { TextareaAutosize } from '@mui/material';
import { Dialog, DialogRef } from '../Dialog';

const DialogErrorFallback = ({ error, resetErrorBoundary }: FallbackProps) => {
  const errorDialogRef = useRef<DialogRef>(null);

  useEffect(() => {
    if (errorDialogRef.current) {
      errorDialogRef.current.open(
        <>
          <p>{error.message}</p>
          <TextareaAutosize value={error.stack} minRows={3} style={{ width: '100%' }} />
        </>,
      );
    }
  }, [errorDialogRef]);

  return <Dialog title="Error occured" ref={errorDialogRef} onClose={resetErrorBoundary} />;
};

const DialogErrorBoundary = ({ children }: { children: React.ReactNode }) => {
  return <ErrorBoundary FallbackComponent={DialogErrorFallback}>{children}</ErrorBoundary>;
};

export default DialogErrorBoundary;
