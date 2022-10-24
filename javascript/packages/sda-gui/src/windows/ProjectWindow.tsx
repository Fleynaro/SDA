import React from 'react';
import { ProjectWindowPayload } from 'sda-electron/api/window';
import useWindowTitle from '../hooks/useWindowTitile';

export default function ProjectWindow({ projectId }: ProjectWindowPayload) {
  useWindowTitle('Project');
  return <div>Your project</div>;
}
