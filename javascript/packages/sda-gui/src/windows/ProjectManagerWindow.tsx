import React, { useCallback } from 'react';
import Button from '@mui/material/Button';
import { getProjectApi, RecentProject, RecentProjectClassName } from 'sda-electron/api/project';
import useList from '../hooks/useList';
import useWindowTitle from '../hooks/useWindowTitile';
import { Box, IconButton, List, ListItem, ListItemButton, ListItemText } from '@mui/material';
import AddIcon from '@mui/icons-material/Add';
import FolderOpenIcon from '@mui/icons-material/FolderOpen';
import DeleteIcon from '@mui/icons-material/Delete';

export default function ProjectManagerWindow() {
  useWindowTitle('Project Manager');
  const recentProjects = useList(getProjectApi(window).getRecentProjects, RecentProjectClassName);

  const onCreateProject = useCallback(() => {
    getProjectApi(window).createProject("D:/SDA_new/build/Debug/test/1", "x86-64");
  }, []);

  const onAddProject = useCallback(() => {
    
  }, []);

  const onOpenProject = useCallback((project: RecentProject) => {
    console.log(project);
  }, []);

  const onDeleteProject = useCallback((project: RecentProject) => {
    console.log(project);
  }, []);

  return (
    <Box>
      <Box>
        <Button variant="text" startIcon={<AddIcon />} onClick={onCreateProject}>
          New
        </Button>
        <Button variant="text" startIcon={<FolderOpenIcon />} onClick={onAddProject}>
          Open
        </Button>
      </Box>
      {recentProjects.length > 0 ? (
        recentProjects.map((project) => (
          <List>
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
          </List>
        ))
      ) : (
        <Box textAlign='center'>
          No recent projects
        </Box>
      )}
    </Box>
  );
}