import { createContext, useContext } from 'react';
import { ObjectId } from 'sda-electron/api/common';

const SdaContext = createContext<ObjectId | null>(null);

interface SdaContextProviderProps {
  contextId: ObjectId;
  children?: React.ReactNode;
}

export const SdaContextProvider = ({ contextId, children }: SdaContextProviderProps) => {
  return <SdaContext.Provider value={contextId}>{children}</SdaContext.Provider>;
};

export const useSdaContextId = () => {
  const contextId = useContext(SdaContext);
  if (contextId === null) {
    throw new Error('SdaContextProvider not found');
  }
  return contextId;
};
