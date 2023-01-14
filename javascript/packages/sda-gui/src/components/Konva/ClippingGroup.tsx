import { Group } from 'react-konva';
import { useStage } from './Stage';

export interface ClippingGroupProps {
  children?: React.ReactNode;
}

// TODO: for optimization purposes, we should only render the children if they are within the stage viewport
export function ClippingGroup({ children }: ClippingGroupProps) {
  const stage = useStage();
  return (
    <Group
      ref={(node) => {
        // Example: https://codesandbox.io/s/react-konva-simple-windowing-render-10000-lines-2hy2u?file=/src/App.js
        node?.visible(false); // condition for clipping: dont show if not within the stage viewport
      }}
    >
      {children}
    </Group>
  );
}
