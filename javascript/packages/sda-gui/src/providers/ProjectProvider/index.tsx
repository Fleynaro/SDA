import { createContext, useContext } from 'react';
import { ObjectId } from 'sda-electron/api/common';

const Project = createContext<ObjectId | null>(null);

interface ProjectProviderProps {
  projectId: ObjectId;
  children?: React.ReactNode;
}

export const ProjectProvider = ({ projectId, children }: ProjectProviderProps) => {
  return <Project.Provider value={projectId}>{children}</Project.Provider>;
};

export const useProjectId = () => {
  const projectId = useContext(Project);
  if (projectId === null) {
    throw new Error('ProjectProvider not found');
  }
  return projectId;
};
