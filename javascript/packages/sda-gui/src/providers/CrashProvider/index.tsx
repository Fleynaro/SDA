import { createContext, useContext, useState } from 'react';

type Error = {
  message: string;
  stack: string;
};

interface CrashContextValue {
  error?: Error;
  setError: (error?: Error) => void;
}

const Crash = createContext<CrashContextValue | null>(null);

interface CrashProviderProps {
  children?: React.ReactNode;
}

export const CrashProvider = ({ children }: CrashProviderProps) => {
  const [error, setError] = useState<Error>();
  return <Crash.Provider value={{ error, setError }}>{children}</Crash.Provider>;
};

export const useCrash = () => {
  const ctx = useContext(Crash);
  if (ctx === null) {
    throw new Error('CrashProvider not found');
  }
  return ctx;
};

export const withCrash = <T, U extends unknown[]>(callback: (...args: U) => Promise<T>) => {
  const { setError } = useCrash();
  return async (...args: U) => {
    try {
      return await callback(...args);
    } catch (e) {
      setError(e as Error);
      console.error(e);
    }
  };
};

export const withCrash_ = (callback: () => Promise<void>) => {
  const { setError } = useCrash();
  return () => {
    (async () => {
      try {
        await callback();
      } catch (e) {
        setError(e as Error);
        console.error(e);
      }
    })();
  };
};
