import { Box, SxProps, Theme } from '@mui/material';
import { createContext, useContext, useEffect, useRef, useState } from 'react';
import { KonvaNodeEvents, Stage, StageProps } from 'react-konva';

type StageSize = {
  width: number;
  height: number;
};

export interface KonvaStageContextValue {
  size: StageSize;
}

const KonvaStageContext = createContext<KonvaStageContextValue | null>(null);

export function useKonvaStage() {
  const context = useContext(KonvaStageContext);
  if (!context) {
    throw new Error('useKonvaStage must be used within a KonvaStage');
  }
  return context;
}

export type KonvaStageProps = KonvaNodeEvents & {
  children?: React.ReactNode;
  sx?: SxProps<Theme>;
};

export function KonvaStage({ children, sx, ...props }: KonvaStageProps) {
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
      <Stage
        width={stageSize.width}
        height={stageSize.height}
        style={{ position: 'absolute' }}
        {...props}
      >
        <KonvaStageContext.Provider value={{ size: stageSize }}>
          {children}
        </KonvaStageContext.Provider>
      </Stage>
    </Box>
  );
}
