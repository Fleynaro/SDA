import { createContext, useContext } from 'react';
import { VFile } from 'sda-electron/api/v-file-system';

export interface VFileSystemContextValue {
  fileToRename?: string;
  setFileToRename: (file: string | undefined) => void;
  renameFile: (file: VFile, newName: string) => void;
  openFile?: (file: VFile) => void;
}

const VFileSystemContext = createContext<VFileSystemContextValue | null>(null);

interface VFileSystemProviderProps {
  value: VFileSystemContextValue;
  children?: React.ReactNode;
}

export const VFileSystemProvider = ({ value, children }: VFileSystemProviderProps) => {
  return <VFileSystemContext.Provider value={value}>{children}</VFileSystemContext.Provider>;
};

export const useVFileSystem = () => {
  const value = useContext(VFileSystemContext);
  if (value === null) {
    throw new Error('useVFileSystem must be used within a VFileSystemProvider');
  }
  return value;
};
