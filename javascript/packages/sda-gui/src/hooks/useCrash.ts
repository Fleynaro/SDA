import { useState, useCallback } from 'react';

export function useCrash() {
  const [, setState] = useState();
  return useCallback(
    (err: unknown) =>
      setState(() => {
        throw err;
      }),
    [],
  );
}

export const withCrash = <T, U extends unknown[]>(callback: (...args: U) => Promise<T>) => {
  const crash = useCrash();
  return async (...args: U) => {
    try {
      return await callback(...args);
    } catch (e) {
      crash(e);
    }
  };
};

export const withCrash_ = (callback: () => Promise<void>) => {
  const crash = useCrash();
  return () => {
    (async () => {
      try {
        await callback();
      } catch (e) {
        crash(e);
      }
    })();
  };
};
