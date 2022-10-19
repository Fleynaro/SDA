import React, { useEffect } from 'react';
import Button from '@mui/material/Button';
import { getProjectApi, Project } from 'sda-electron/api/project';
import { getNotifierApi, ObjectId, ObjectChangeType } from 'sda-electron/api/notifier';
 
export default function App() {
  const [projects, setProjects] = React.useState<Project[]>([]);

  useEffect(() => {
    getProjectApi(window).getProjects().then(setProjects);

    const unsubscribe1 = getNotifierApi(window).subscribeToObjectChanges((id: ObjectId, changeType: ObjectChangeType) => {
      console.log('Object changed', id, changeType);
    });

    const unsubscribe2 = getNotifierApi(window).subscribeToObjectChanges((id: ObjectId, changeType: ObjectChangeType) => {
      if (id.className !== 'Project')
        return;
      getProjectApi(window).getProjects().then(setProjects);
    });

    return () => {
      unsubscribe1();
      unsubscribe2();
    }
  }, []);

  const handleCreateProject = () => {
    getProjectApi(window).createProject("D:/SDA_new/build/Debug/test/1", "x86-64");
  };

  return (
    <div>
      <Button variant="outlined" onClick={handleCreateProject}>Create</Button>
      {projects.map((project) => (
        <div key={project.id.key}>{project.path}</div>
      ))}
    </div>
  );
}