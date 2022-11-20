import { ListItemIcon, ListItemText, Paper, Typography } from '@mui/material';
import { MenuBarTopItem, MenuBar, MenuBarList, MenuBarItem } from 'components/MenuBar';
import { useProjectId } from 'providers/ProjectProvider';
import FileOpenIcon from '@mui/icons-material/FileOpen';

interface ProjectMenuBarProps {
  f?: number;
}

export default function ProjectMenuBar(props: ProjectMenuBarProps) {
  const projectId = useProjectId();
  return (
    <MenuBar>
      <MenuBarTopItem label="File">
        <Paper sx={{ width: 200, maxWidth: '100%' }}>
          <MenuBarList>
            <MenuBarItem>
              <ListItemIcon>
                <FileOpenIcon fontSize="small" />
              </ListItemIcon>
              <ListItemText>Open</ListItemText>
              <Typography variant="body2" color="text.secondary">
                Ctrl + O
              </Typography>
            </MenuBarItem>
          </MenuBarList>
        </Paper>
      </MenuBarTopItem>
      <MenuBarTopItem label="Edit">6</MenuBarTopItem>
      <MenuBarTopItem label="View">7</MenuBarTopItem>
      <MenuBarTopItem label="Help">8</MenuBarTopItem>
    </MenuBar>
  );
}
