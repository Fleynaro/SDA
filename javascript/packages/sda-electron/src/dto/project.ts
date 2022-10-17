import { Project } from 'sda';
import { Project as ProjectDTO } from '@api/project';
import { toContextDTO } from './context';

export const toProjectDTO = (project: Project): ProjectDTO => {
    return {
        hashId: project.hashId,
        path: project.path,
        context: toContextDTO(project.context),
    };
};