import { createContext, useContext } from 'react';
import { Jump } from 'sda-electron/api/image';

export interface ImageContentContextValue {
  goToOffset: (offset: number) => Promise<boolean>;
  selectedJump?: Jump;
  setSelectedJump?: (jump?: Jump) => void;
  rowSelection: {
    selectedRows: number[];
    setSelectedRows: (rows: number[]) => void;
    firstSelectedRow?: number;
    setFirstSelectedRow?: (row?: number) => void;
    lastSelectedRow?: number;
    setLastSelectedRow?: (row?: number) => void;
  };
  style: {
    // dynamic style
    rowWidth: number;
  };
}

export const ImageContentContext = createContext<ImageContentContextValue | null>(null);

export const useImageContentContext = () => {
  const ctx = useContext(ImageContentContext);
  if (!ctx) throw new Error('ImageContentContext is not set');
  return ctx;
};
