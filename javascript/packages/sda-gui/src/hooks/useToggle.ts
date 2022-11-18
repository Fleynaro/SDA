import { useState, useCallback } from 'react';

export const useToggle = (initialValue: boolean) => {
  const [value, setValue] = useState(initialValue);
  const toggle = useCallback(() => setValue((value) => !value), []);
  return [value, toggle] as const;
};

export const useToggleList = <T>(initialValue: T[]) => {
  const [value, setValue] = useState(initialValue);
  const toggle = useCallback(
    (item: T) => {
      setValue((value) => {
        if (value.includes(item)) {
          return value.filter((i) => i !== item);
        }
        return [...value, item];
      });
    },
    [setValue],
  );
  return [value, setValue, toggle] as const;
};
