import { Box, SxProps, Theme } from '@mui/material';
import { useEffect, useRef, useState } from 'react';
import { Stage } from 'react-konva';

export interface KonvaStageProps {
  children?: React.ReactNode;
  sx?: SxProps<Theme>;
}

export function KonvaStage({ children, sx }: KonvaStageProps) {
  const stageContainerRef = useRef<HTMLDivElement>(null);
  const [stageSize, setStageSize] = useState({ width: 0, height: 0 });

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
      <Stage width={stageSize.width} height={stageSize.height} style={{ position: 'absolute' }}>
        {children}
      </Stage>
    </Box>
  );
}
