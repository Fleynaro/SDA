import { useState, useCallback } from 'react';

export default function useCrash() {
  const [, setState] = useState();
  return useCallback(
    (err: unknown) =>
      setState(() => {
        throw err;
      }),
    [],
  );
}
