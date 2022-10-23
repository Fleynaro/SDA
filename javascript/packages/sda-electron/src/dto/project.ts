import { Project } from 'sda';
import {
    Project as ProjectDTO,
    RecentProject as RecentProjectDTO,
    RecentProjectClassName,
    ProjectClassName
} from '../api/project';
import { ObjectId } from '../api/common';
import { toContextId } from './context';
import { basename as pathBasename } from 'path';

export const toProjectId = (project: Project): ObjectId => {
    return {
        key: project.hashId.toString(),
        className: ProjectClassName,
    };
};

export const toProjectDTO = (project: Project): ProjectDTO => {
    return {
        id: toProjectId(project),
        path: project.path,
        context: toContextId(project.context),
    };
};

export const toRecentProjectDTO = (path: string): RecentProjectDTO => {
    return {
        id: {
            key: path,
            className: RecentProjectClassName,
        },
        path: path,
        name: pathBasename(path),
    };
}