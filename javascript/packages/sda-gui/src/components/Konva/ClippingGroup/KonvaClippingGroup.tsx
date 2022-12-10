import { Group } from 'react-konva';
import { useKonvaStage } from '../Stage';

export interface KonvaClippingGroupProps {
  children?: React.ReactNode;
}

// TODO: for optimization purposes, we should only render the children if they are within the stage viewport
export function KonvaClippingGroup({ children }: KonvaClippingGroupProps) {
  const stage = useKonvaStage();
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
