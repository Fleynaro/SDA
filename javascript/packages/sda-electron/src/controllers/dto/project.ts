import { Project } from 'sda';
import {
    Project as ProjectDTO,
    RecentProject as RecentProjectDTO,
    RecentProjectClassName,
} from '../../api/project';
import { ObjectId } from '../../api/common';
import { toContextId } from './context';
import { basename as pathBasename } from 'path';
import { toHash, toId } from '../../utils/common';

export const toProjectDTO = (project: Project): ProjectDTO => {
    return {
        id: toId(project),
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
