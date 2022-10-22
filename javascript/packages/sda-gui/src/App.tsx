import React, { useEffect, useState } from 'react';
import { getEventApi } from 'sda-electron/api/event';
import { WindowName } from 'sda-electron/api/window';
import ProjectManagerWindow from './windows/ProjectManagerWindow';
import ProjectWindow from './windows/ProjectWindow';

interface WindowInfo {
  name: WindowName;
  payload: any;
}

export default function App() {
  const [windowInfo, setWindowInfo] = useState<WindowInfo>();

  useEffect(() => {
    const unsubscribe = getEventApi(window).subscribeToWindowOpenEvent((name: WindowName, payload: any) => {
      setWindowInfo({
        name,
        payload
      });
    });

    return () => {
      unsubscribe();
    }
  });

  return (
    <div>
      {windowInfo ? (
        (windowInfo.name === WindowName.ProjectManager && <ProjectManagerWindow {...windowInfo.payload}/>) ||
        (windowInfo.name === WindowName.Project && <ProjectWindow {...windowInfo.payload}/>)
      ) : (
        <div>Loading...</div>
      )}
    </div>
  );
}