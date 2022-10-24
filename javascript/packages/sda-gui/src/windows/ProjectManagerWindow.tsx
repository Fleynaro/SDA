import { useCallback, useRef } from 'react';
import {
  Box,
  Button,
  IconButton,
  List,
  ListItem,
  ListItemButton,
  ListItemText,
} from '@mui/material';
import AddIcon from '@mui/icons-material/Add';
import FolderOpenIcon from '@mui/icons-material/FolderOpen';
import DeleteIcon from '@mui/icons-material/Delete';
import { Dialog, DialogRef } from '../components/Dialog';
import { CreateProjectFormRef, CreateProjectForm } from '../components/CreateProjectForm';
import useList from '../hooks/useList';
import useWindowTitle from '../hooks/useWindowTitile';
import {
  getProjectApi,
  RecentProject,
  ProjectClassName,
  RecentProjectClassName,
} from 'sda-electron/api/project';
import { getWindowApi } from 'sda-electron/api/window';

export default function ProjectManagerWindow() {
  useWindowTitle('Project Manager');
  const recentProjects = useList(getProjectApi().getRecentProjects, RecentProjectClassName);
  const activeProjects = useList(getProjectApi().getActiveProjects, ProjectClassName);

  const createProjectDialogRef = useRef<DialogRef>(null);
  const createProjectFormRef = useRef<CreateProjectFormRef>(null);
  const deleteProjectDialogRef = useRef<DialogRef>(null);

  const onOpenCreateProjectDialog = useCallback(() => {
    createProjectDialogRef.current?.open(
      <Box component="form" noValidate>
        <CreateProjectForm ref={createProjectFormRef} />
      </Box>,
      <Button
        onClick={() => {
          createProjectFormRef.current?.create();
          createProjectDialogRef.current?.close();
        }}
      >
        Create
      </Button>,
    );
  }, []);

  const onAddProject = useCallback(async () => {
    const paths = await getWindowApi().openFilePickerDialog(true, false);
    if (paths.length > 0) {
      const projectPath = paths[0];
      const project = await getProjectApi().openProject(projectPath);
    }
  }, []);

  const onOpenProject = useCallback(async (recentProject: RecentProject) => {
    const project = await getProjectApi().openProject(recentProject.path);
    await getWindowApi().openProjectWindow({ projectId: project.id });
  }, []);

  const onDeleteProject = useCallback((recentProject: RecentProject) => {
    deleteProjectDialogRef.current?.open(
      <Box>
        <p>Are you sure you want to delete this project?</p>
        <p>{recentProject.path}</p>
      </Box>,
      <Button
        onClick={() => {
          //getProjectApi().deleteProject(project.path);
          deleteProjectDialogRef.current?.close();
        }}
      >
        Delete
      </Button>,
    );
  }, []);

  return (
    <>
      <Box>
        <Button variant="text" startIcon={<AddIcon />} onClick={onOpenCreateProjectDialog}>
          New
        </Button>
        <Button variant="text" startIcon={<FolderOpenIcon />} onClick={onAddProject}>
          Open
        </Button>
      </Box>
      {recentProjects.length > 0 ? (
        <List>
          {recentProjects.map((project) => {
            const isOpened = activeProjects.some((p) => p.path === project.path);
            return (
              <ListItem
                key={project.id.key}
                secondaryAction={
                  <IconButton
                    edge="end"
                    aria-label="delete"
                    onClick={() => onDeleteProject(project)}
                    disabled={!isOpened}
                  >
                    <DeleteIcon />
                  </IconButton>
                }
                disablePadding
              >
                <ListItemButton onClick={() => onOpenProject(project)} disabled={!isOpened}>
                  <ListItemText primary={project.name} secondary={project.path} />
                </ListItemButton>
              </ListItem>
            );
          })}
        </List>
      ) : (
        <Box textAlign="center">No recent projects</Box>
      )}
      <Dialog title="New project" showCancelButton={true} ref={createProjectDialogRef} />
      <Dialog title="Delete project" showCancelButton={true} ref={deleteProjectDialogRef} />
    </>
  );
}
