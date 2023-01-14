import { Box, SxProps, Theme } from '@mui/material';
import { createContext, useContext, useEffect, useRef, useState } from 'react';
import { KonvaNodeEvents, Stage as KonvaStage } from 'react-konva';

export type StageSize = {
  width: number;
  height: number;
};

export interface StageContextValue {
  size: StageSize;
}

const StageContext = createContext<StageContextValue | null>(null);

export function useStage() {
  const context = useContext(StageContext);
  if (!context) {
    throw new Error('useStage must be used within a Stage');
  }
  return context;
}

export type StageProps = KonvaNodeEvents & {
  children?: React.ReactNode;
  sx?: SxProps<Theme>;
};

export function Stage({ children, sx, ...props }: StageProps) {
  const stageContainerRef = useRef<HTMLDivElement>(null);
  const [stageSize, setStageSize] = useState<StageSize>({ width: 0, height: 0 });

  useEffect(() => {
    const updater = setInterval(() => {
      const container = stageContainerRef.current;
      if (container) {
        const width = container.clientWidth;
        const height = container.clientHeight;
        if (width !== stageSize.width || height !== stageSize.height) {
          setStageSize({ width, height });
        }
      }
    }, 100);
    return () => clearInterval(updater);
  }, [stageContainerRef, stageSize]);

  return (
    <Box sx={sx} ref={stageContainerRef}>
      <KonvaStage
        width={stageSize.width}
        height={stageSize.height}
        style={{ position: 'absolute' }}
        {...props}
      >
        <StageContext.Provider value={{ size: stageSize }}>{children}</StageContext.Provider>
      </KonvaStage>
    </Box>
  );
}
