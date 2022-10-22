import React, { useEffect } from 'react';
import Button from '@mui/material/Button';
import { getProjectApi, Project } from 'sda-electron/api/project';
import { getEventApi } from 'sda-electron/api/event';
import { ObjectId, ObjectChangeType } from 'sda-electron/api/common';
 
export default function ProjectManagerWindow() {
  const [projects, setProjects] = React.useState<Project[]>([]);

  useEffect(() => {
    getProjectApi(window).getProjects().then(setProjects);

    const unsubscribe = getEventApi(window).subscribeToObjectChangeEvent((id: ObjectId, changeType: ObjectChangeType) => {
      if (id.className !== 'Project')
        return;
      getProjectApi(window).getProjects().then(setProjects);
    });

    return () => {
      unsubscribe();
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