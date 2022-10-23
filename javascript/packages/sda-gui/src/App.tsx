import React, { useState, useEffect } from 'react';
import { getWindowApi, WindowName, WindowInfo } from 'sda-electron/api/window';
import ProjectManagerWindow from './windows/ProjectManagerWindow';
import ProjectWindow from './windows/ProjectWindow';

export default function App() {
  const [windowToShow, setWindowToShow] = useState<WindowInfo>();

  useEffect(() => {
    getWindowApi(window).getWindowInfo().then(setWindowToShow);
  }, []);

  return (
    <div>
      {windowToShow ? (
        (windowToShow.name === WindowName.ProjectManager && <ProjectManagerWindow {...windowToShow.payload}/>) ||
        (windowToShow.name === WindowName.Project && <ProjectWindow {...windowToShow.payload}/>)
      ) : (
        <div>Loading...</div>
      )}
    </div>
  );
}