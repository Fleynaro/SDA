import { useState } from 'react';
import { useEffect } from './hooks/reactWrappedHooks';
import { Box, LinearProgress } from '@mui/material';
import DialogErrorBoundary from './components/DialogErrorBoundary';
import { getWindowApi, WindowName, WindowInfo } from 'sda-electron/api/window';
import ProjectManagerWindow from './windows/ProjectManagerWindow';
import ProjectWindow from './windows/ProjectWindow';

export default function App() {
  const [windowToShow, setWindowToShow] = useState<WindowInfo>();

  useEffect(async () => {
    setWindowToShow(await getWindowApi().getWindowInfo());
  }, []);

  return (
    <>
      <DialogErrorBoundary>
        {windowToShow ? (
          (windowToShow.name === WindowName.ProjectManager && (
            <ProjectManagerWindow {...windowToShow.payload} />
          )) ||
          (windowToShow.name === WindowName.Project && <ProjectWindow {...windowToShow.payload} />)
        ) : (
          <Box sx={{ width: '100%' }}>
            <LinearProgress />
          </Box>
        )}
      </DialogErrorBoundary>
    </>
  );
}
