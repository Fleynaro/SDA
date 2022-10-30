import React from 'react';
import { ProjectWindowPayload } from 'sda-electron/api/window';
import { getProjectApi } from 'sda-electron/api/project';
import useWindowTitle from '../hooks/useWindowTitile';
import useObject from '../hooks/useObject';

export default function ProjectWindow({ projectId }: ProjectWindowPayload) {
  useWindowTitle('Project');
  const project = useObject(getProjectApi().getActiveProject, projectId);
  return <div>Your project {project && project.path}</div>;
}
