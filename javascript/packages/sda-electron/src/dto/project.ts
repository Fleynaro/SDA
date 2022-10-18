import { Project } from 'sda';
import { Project as ProjectDTO } from '../api/project';
import { ObjectId } from '../api/common';
import { toContextId } from './context';

export const toProjectId = (project: Project): ObjectId => {
    return {
        key: project.hashId.toString(),
        className: 'Project',
    };
};

export const toProjectDTO = (project: Project): ProjectDTO => {
    return {
        id: toProjectId(project),
        path: project.path,
        context: toContextId(project.context),
    };
};