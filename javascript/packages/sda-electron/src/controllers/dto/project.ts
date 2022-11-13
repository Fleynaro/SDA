import { Project } from 'sda';
import {
    Project as ProjectDTO,
    RecentProject as RecentProjectDTO,
    RecentProjectClassName,
    ProjectClassName
} from '../../api/project';
import { ObjectId } from '../../api/common';
import { toContextId } from './context';
import { basename as pathBasename } from 'path';
import { toHash } from '../../utils/common';

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
        contextId: toContextId(project.context),
    };
};

export const toProject = (id: ObjectId): Project => {
    const project = Project.Get(toHash(id));
    if (!project) {
        throw new Error(`Project ${id.key} does not exist`);
    }
    return project;
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
