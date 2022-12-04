import React from 'react';
import useCrash from './useCrash';

export function useEffect<T>(effect: () => T, deps?: React.DependencyList) {
  const crash = useCrash();
  React.useEffect(() => {
    (async function () {
      try {
        return await effect();
      } catch (e) {
        crash(e);
      }
    })();
  }, deps);
}

// eslint-disable-next-line @typescript-eslint/ban-types
export function useCallback<T extends Function>(callback: T, deps: React.DependencyList) {
  const crash = useCrash();
  return React.useCallback(
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    async (...args: any[]) => {
      try {
        return await callback(...args);
      } catch (e) {
        crash(e);
      }
    },
    deps,
  ) as unknown as T;
}

export function useMemo<T>(factory: () => T, deps: React.DependencyList) {
  const crash = useCrash();
  return React.useMemo(async () => {
    try {
      return await factory();
    } catch (e) {
      crash(e);
    }
  }, deps);
}

export function useImperativeHandle<T, R extends T>(
  ref: React.Ref<T> | undefined,
  init: () => R,
  deps?: React.DependencyList,
): void {
  const crash = useCrash();
  React.useImperativeHandle<T, R>(
    ref,
    () => {
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      const obj = init() as any;
      if (typeof obj === 'object') {
        Object.keys(obj).forEach((key) => {
          const value = obj[key];
          if (typeof value === 'function') {
            // eslint-disable-next-line @typescript-eslint/no-explicit-any
            obj[key] = async (...args: any[]) => {
              try {
                return await value(...args);
              } catch (e) {
                crash(e);
              }
            };
          }
        });
      }
      return obj;
    },
    deps,
  );
}
