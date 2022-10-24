import { useCallback, useRef } from 'react';
import Button from '@mui/material/Button';
import { getProjectApi, RecentProject, RecentProjectClassName } from 'sda-electron/api/project';
import useList from '../hooks/useList';
import useWindowTitle from '../hooks/useWindowTitile';
import { Dialog, DialogRef } from '../components/Dialog';
import { CreateProjectFormRef, CreateProjectForm } from '../components/CreateProjectForm';
import { Box, IconButton, List, ListItem, ListItemButton, ListItemText } from '@mui/material';
import AddIcon from '@mui/icons-material/Add';
import FolderOpenIcon from '@mui/icons-material/FolderOpen';
import DeleteIcon from '@mui/icons-material/Delete';

export default function ProjectManagerWindow() {
  useWindowTitle('Project Manager');
  const recentProjects = useList(getProjectApi(window).getRecentProjects, RecentProjectClassName);

  const dialogRef = useRef<DialogRef>(null);
  const createProjectFormRef = useRef<CreateProjectFormRef>(null);

  const onOpenCreateProjectDialog = useCallback(() => {
    //getProjectApi(window).createProject("D:/SDA_new/build/Debug/test/1", "x86-64");
    dialogRef.current?.open(
      <Box component="form" noValidate>
        <CreateProjectForm ref={createProjectFormRef} />
      </Box>,
      <Button onClick={() => createProjectFormRef.current?.create()}>Create</Button>,
    );
  }, []);

  const onAddProject = useCallback(() => {}, []);

  const onOpenProject = useCallback((project: RecentProject) => {
    console.log(project);
  }, []);

  const onDeleteProject = useCallback((project: RecentProject) => {
    console.log(project);
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
          {recentProjects.map((project) => (
            <ListItem
              key={project.id.key}
              secondaryAction={
                <IconButton edge="end" aria-label="delete" onClick={() => onDeleteProject(project)}>
                  <DeleteIcon />
                </IconButton>
              }
              disablePadding
            >
              <ListItemButton onClick={() => onOpenProject(project)}>
                <ListItemText primary={project.name} secondary={project.path} />
              </ListItemButton>
            </ListItem>
          ))}
        </List>
      ) : (
        <Box textAlign="center">No recent projects</Box>
      )}
      <Dialog title="New project" showCancelButton={true} ref={dialogRef} />
    </>
  );
}
