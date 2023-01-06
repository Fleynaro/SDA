import { useState, useEffect } from 'react';
import { Box, LinearProgress } from '@mui/material';
import DialogCrash from './components/DialogCrash';
import { getWindowApi, WindowName, WindowInfo } from 'sda-electron/api/window';
import ProjectManagerWindow from './windows/ProjectManagerWindow';
import ProjectWindow from './windows/ProjectWindow';
import { withCrash_ } from 'providers/CrashProvider';

export default function App() {
  const [windowToShow, setWindowToShow] = useState<WindowInfo>();

  useEffect(
    withCrash_(async () => {
      setWindowToShow(await getWindowApi().getWindowInfo());
    }),
    [],
  );

  return (
    <>
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
      <DialogCrash />
    </>
  );
}
